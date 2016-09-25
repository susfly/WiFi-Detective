#ifndef _WLAN_NODE_H_
#define _WLAN_NODE_H_

typedef enum wlan_event_id_e {
  WLAN_ACM_SCAN_COMPLETE,
  WLAN_ACM_CONNECT_COMPLETE,
  WLAN_ACM_DISCONNECTED,
  WLAN_MSM_ROAMING_END,
  WLAN_MSM_ADAPTER_REMOVAL,
  WLAN_EVENT_ID_MAX,
} wlan_event_id_t;

#endif /* _WLAN_NODE_H_ */
