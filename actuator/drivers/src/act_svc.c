#include "act_svc/act_svc.h"

BT_GATT_SERVICE_DEFINE(act_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ACTS),
);
