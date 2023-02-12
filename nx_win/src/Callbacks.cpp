#include "Stable.h"
#include "NXExtensionImpl.h"

UF_MB_cb_status_t NXExtensionImpl::RunEditorAction(
    UF_MB_widget_t, UF_MB_data_t, UF_MB_activated_button_p_t )
{
    NXExtensionImpl::Instance().RunEditor();
    return UF_MB_CB_CONTINUE;
}
