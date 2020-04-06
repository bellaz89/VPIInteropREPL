#pragma once
#include<vpi_user.h>

typedef struct {

    vpiHandle (*vpi_register_cb)(p_cb_data);
    PLI_INT32 (*vpi_control)(PLI_INT32, ...);
    vpiHandle (*vpi_handle)(PLI_INT32, vpiHandle);
    vpiHandle (*vpi_handle_by_name)(PLI_BYTE8*, vpiHandle);
    void (*vpi_get_value)(vpiHandle, p_vpi_value);
    vpiHandle (*vpi_put_value)(vpiHandle, p_vpi_value, p_vpi_time, PLI_INT32);
    PLI_INT32 (*vpi_get) (PLI_INT32, vpiHandle);
    PLI_BYTE8* (*vpi_get_str)(PLI_INT32, vpiHandle);
    vpiHandle (*vpi_scan)(vpiHandle);
    vpiHandle (*vpi_iterate)(PLI_INT32, vpiHandle);
    PLI_INT32 (*vpi_printf)(PLI_BYTE8*, ...);
    PLI_INT32 (*vpi_chk_error)(p_vpi_error_info);

} VpiFnTable;
