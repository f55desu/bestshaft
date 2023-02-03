#include "TopWindow.h"

//Code in this module is specific to Windows

TopWindow::TopWindow( HWND parent ) : QWidget(),
    hParent( parent ), reenable_parent( false ), prevFocus( 0x0 )
{
    ::SetWindowLong( winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
    ::SetParent( winId(), parent );

    QEvent e( QEvent::EmbeddingControl );
    QApplication::sendEvent( this, &e );
}

TopWindow::~TopWindow()
{
}

HWND TopWindow::parentWindow() const
{
    return hParent;
}

void TopWindow::childEvent( QChildEvent* e )
{
    QObject* obj = e->child();

    if ( obj->isWidgetType() )
    {
        if ( e->added() )
        {
            if ( obj->isWidgetType() )
                obj->installEventFilter( this );
        }
        else if ( e->removed() && reenable_parent )
        {
            reenable_parent = false;
            ::EnableWindow( hParent, true );
            obj->removeEventFilter( this );
        }
    }

    QWidget::childEvent( e );
}

void TopWindow::saveFocus()
{
    if ( !prevFocus )
        prevFocus = ::GetFocus();

    if ( !prevFocus )
        prevFocus = parentWindow();
}

void TopWindow::resetFocus()
{
    if ( prevFocus )
        ::SetFocus( prevFocus );
    else
        ::SetFocus( parentWindow() );
}

bool TopWindow::eventFilter( QObject* o, QEvent* e )
{
    QWidget* w = ( QWidget* )o;

    switch ( e->type() )
    {
        case QEvent::WindowDeactivate:
            if ( w->isModal() && w->isHidden() )
                BringWindowToTop( hParent );

            break;

        case QEvent::Hide:
            if ( reenable_parent )
            {
                EnableWindow( hParent, true );
                reenable_parent = false;
            }

            resetFocus();
            /*if (w->testAttribute(Qt::WA_DeleteOnClose) && w->isWindow())
                deleteLater();*/
            break;

        case QEvent::Show:
            if ( w->isWindow() )
            {
                saveFocus();
                hide();

                if ( w->isModal() && !reenable_parent )
                {
                    EnableWindow( hParent, false );
                    reenable_parent = true;
                }
            }

            break;

        case QEvent::Close:
            ::SetActiveWindow( hParent );
            /*if (w->testAttribute(Qt::WA_DeleteOnClose))
                deleteLater();*/
            break;

        default:
            break;
    }

    return QWidget::eventFilter( o, e );
}
