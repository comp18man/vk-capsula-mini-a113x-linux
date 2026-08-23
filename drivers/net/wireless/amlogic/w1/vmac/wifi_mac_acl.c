/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac ACL module ,access control module,
 *
 *
 ****************************************************************************************
 */

#include "wifi_mac_com.h"

int my_mod_use = 0;


enum
{
    ACL_POLICY_OPEN     = 0,
    ACL_POLICY_ALLOW    = 1,
    ACL_POLICY_DENY     = 2,
};


struct NETWIFI_S_ACL_INFO
{
    struct list_head   acl_list;
    struct list_head       acl_hash;
    unsigned char     acl_macaddr[WIFINET_ADDR_LEN];
};
struct NETWIFI_S_ACL
{
    spinlock_t          as_lock;
    int                     as_policy;
    struct list_head   as_list;
    struct list_head       as_hash[ACL_HASHSIZE];
};




static  void acl_free_all_locked(struct NETWIFI_S_ACL *);
static int
acl_attach(struct wlan_net_vif *wnet_vif)
{
    struct NETWIFI_S_ACL *aclpriv;
    int Num=0;
    MY_MOD_INC_USE(THIS_MODULE, return 0);

    aclpriv = (struct NETWIFI_S_ACL *)NET_MALLOC(sizeof(struct NETWIFI_S_ACL), GFP_KERNEL, "acl_attach.aclpriv");
    if (aclpriv == NULL)
    {
        MY_MOD_DEC_USE(THIS_MODULE);
        return 0;
    }
    ACL_LOCK_INIT(aclpriv, "aclpriv");
    INIT_LIST_HEAD(&aclpriv->as_list);
    for (Num = 0; Num < ACL_HASHSIZE; Num++)
    {
        INIT_LIST_HEAD(&aclpriv->as_hash[Num]);
    }

    aclpriv->as_policy = ACL_POLICY_OPEN;
    wnet_vif->vm_aclpriv = aclpriv;
    return 1;
}

static void
acl_detach(struct wlan_net_vif *wnet_vif)
{
    struct NETWIFI_S_ACL *aclpriv = wnet_vif->vm_aclpriv;

    ACL_LOCK(aclpriv);
    acl_free_all_locked(aclpriv);
    ACL_UNLOCK(aclpriv);
    wnet_vif->vm_aclpriv = NULL;
    ACL_LOCK_DESTROY(aclpriv);
    FREE(wnet_vif->vm_aclpriv,"wnet_vif->vm_aclpriv");

    MY_MOD_DEC_USE(THIS_MODULE);
}

static __inline struct NETWIFI_S_ACL_INFO *
_find_acl(struct NETWIFI_S_ACL *aclpriv, const unsigned char *macaddr)
{
    struct NETWIFI_S_ACL_INFO *aclinfo;
    int hash;

    hash = ACL_HASH(macaddr);
    list_for_each_entry(aclinfo, &aclpriv->as_hash[hash], acl_hash)
    {
        if (WIFINET_ADDR_EQ(aclinfo->acl_macaddr, macaddr))
            return aclinfo;
    }
    return NULL;
}

static void
_acl_free(struct NETWIFI_S_ACL *aclpriv, struct NETWIFI_S_ACL_INFO *aclinfo)
{
    ACL_LOCK_ASSERT(aclpriv);

    list_del_init(&aclinfo->acl_list);
    list_del_init(&aclinfo->acl_hash);
//        LIST_REMOVE(NETWIFI_S_ACL_INFO, acl_hash);
    FREE(aclinfo,"aclinfo");
}

static int
acl_check(struct wlan_net_vif *wnet_vif, const unsigned char mac[WIFINET_ADDR_LEN])
{
    struct NETWIFI_S_ACL *aclpriv = wnet_vif->vm_aclpriv;

    switch (aclpriv->as_policy)
    {
        case ACL_POLICY_OPEN:
            return 1;
        case ACL_POLICY_ALLOW:
            return _find_acl(aclpriv, mac) != NULL;
        case ACL_POLICY_DENY:
            return _find_acl(aclpriv, mac) == NULL;
    }
    return 0;       /* should not happen */
}

static int
acl_add(struct wlan_net_vif *wnet_vif, const unsigned char mac[WIFINET_ADDR_LEN])
{
    struct NETWIFI_S_ACL *aclpriv = wnet_vif->vm_aclpriv;
    struct NETWIFI_S_ACL_INFO *aclinfo, *new;
    int hash;

    new = (struct NETWIFI_S_ACL_INFO *)NET_MALLOC(sizeof(struct NETWIFI_S_ACL_INFO), GFP_KERNEL, "acl_add.new");
    if (new == NULL)
    {
        return ENOMEM;
    }

    ACL_LOCK(aclpriv);
    hash = ACL_HASH(mac);
    list_for_each_entry(aclinfo, &aclpriv->as_hash[hash], acl_hash)
    {
        if (WIFINET_ADDR_EQ(aclinfo->acl_macaddr, mac))
        {
            ACL_UNLOCK(aclpriv);
            FREE(new,"new");
            return EEXIST;
        }
    }
    WIFINET_ADDR_COPY(new->acl_macaddr, mac);
    list_add_tail(&new->acl_list, &aclpriv->as_list);

    list_add(&new->acl_hash, &aclpriv->as_hash[hash]);
    ACL_UNLOCK(aclpriv);

    return 0;
}

static int
acl_remove(struct wlan_net_vif *wnet_vif, const unsigned char mac[WIFINET_ADDR_LEN])
{
    struct NETWIFI_S_ACL *aclpriv = wnet_vif->vm_aclpriv;
    struct NETWIFI_S_ACL_INFO *aclinfo;

    ACL_LOCK(aclpriv);
    aclinfo = _find_acl(aclpriv, mac);
    if (aclinfo != NULL)
        _acl_free(aclpriv, aclinfo);
    ACL_UNLOCK(aclpriv);

    return (aclinfo == NULL ? ENOENT : 0);
}

static void
acl_free_all_locked(struct NETWIFI_S_ACL *aclpriv)
{
    struct NETWIFI_S_ACL_INFO *aclinfo;

    ACL_LOCK(aclpriv);
    while (!list_empty(&aclpriv->as_list))
    {
        aclinfo = list_first_entry(&aclpriv->as_list, struct NETWIFI_S_ACL_INFO, acl_list);
        _acl_free(aclpriv, aclinfo);
    }
    ACL_UNLOCK(aclpriv);
}

static int
acl_free_all(struct wlan_net_vif *wnet_vif)
{

    acl_free_all_locked(wnet_vif->vm_aclpriv);

    return 0;
}

static int
acl_setpolicy(struct wlan_net_vif *wnet_vif, int policy)
{
    struct NETWIFI_S_ACL *aclpriv = wnet_vif->vm_aclpriv;

    switch (policy)
    {
        case WIFINET_MACCMD_POLICY_OPEN:
            aclpriv->as_policy = ACL_POLICY_OPEN;
            break;
        case WIFINET_MACCMD_POLICY_ALLOW:
            aclpriv->as_policy = ACL_POLICY_ALLOW;
            break;
        case WIFINET_MACCMD_POLICY_DENY:
            aclpriv->as_policy = ACL_POLICY_DENY;
            break;
        default:
            return EINVAL;
    }
    return 0;
}

static int
acl_getpolicy(struct wlan_net_vif *wnet_vif)
{
    struct NETWIFI_S_ACL *aclpriv = wnet_vif->vm_aclpriv;

    return aclpriv->as_policy;
}

static const struct wifi_mac_actuator mac_acl=
{
    .iac_name   = "acl",
    .iac_attach = acl_attach,
    .iac_detach = acl_detach,
    .iac_check  = acl_check,
    .iac_add    = acl_add,
    .iac_remove = acl_remove,
    .iac_flush  = acl_free_all,
    .iac_setpolicy  = acl_setpolicy,
    .iac_getpolicy  = acl_getpolicy,
};


struct wifi_mac_actuator * wifi_mac_acl_attach(void)
{
    return (struct wifi_mac_actuator *)&mac_acl;
}

void wifi_mac_acl_deattach(void)
{
}


