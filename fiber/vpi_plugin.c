#include <vpi_user.h>

#include "vpiFnTable.h"


void entry_point() {

    void (*entry_point_ptr)(VpiFnTable) = (void(*)(VpiFnTable)) VPI_ENTRY_POINT_PTR;
    VpiFnTable table;

    table.vpi_register_cb    = &vpi_register_cb;
    table.vpi_control        = &vpi_control;
    table.vpi_handle         = &vpi_handle;
    table.vpi_handle_by_name = &vpi_handle_by_name;
    table.vpi_get_value      = &vpi_get_value;
    table.vpi_put_value      = &vpi_put_value;
    table.vpi_get            = &vpi_get;
    table.vpi_get_str        = &vpi_get_str;
    table.vpi_scan           = &vpi_scan;
    table.vpi_iterate        = &vpi_iterate;
    table.vpi_printf         = &vpi_printf;
    table.vpi_chk_error      = &vpi_chk_error;

    entry_point_ptr(table);
}

void (*vlog_startup_routines[]) () = {
    entry_point,
    0
};
