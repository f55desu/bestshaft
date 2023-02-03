#include "Stable.h"
#include "ExtensionImpl.h"

ExtensionImpl ExtensionImpl::m_instance;

ExtensionImpl::ExtensionImpl() :
    BaseExtension()
{
}

void ExtensionImpl::Initialize()
{
    //Call base class initialization first
    BaseExtension::Initialize();

    // Initialize the API environment here
}

QWidget* ExtensionImpl::GetTopWindow()
{
    return m_topWindow;
}

void ExtensionImpl::CheckContextIsValid()
{
    m_scriptEngine->throwError(
        QString( "Work part doesn't exist. Create new or open existing part model." ) );
}

QJSValue ExtensionImpl::SKETCH_Open( QString skt_name, QJSValue csys )
{
    Q_UNUSED( skt_name );
    Q_UNUSED( csys );

    return QJSValue();
}

QJSValue ExtensionImpl::SKETCH_Close( QJSValue sketch )
{
    Q_UNUSED( sketch );

    return QJSValue();
}

/*QJSValue ExtensionImpl::SELECT_Face(double Xp, double Yp, double Zp,
                                        double Xr, double Yr, double Zr,
                                        int number)
{
    return QJSValue();
}

QJSValue ExtensionImpl::SELECT_Edge(double Xp, double Yp, double Zp, double Xr, double Yr, double Zr, int number)
{
    return QJSValue();
}*/

QJSValue ExtensionImpl::SELECT_Object( double /*xPoint*/, double /*yPoint*/, double /*zPoint*/,
                                       TopologyPriority /*topologyPriority = TP_ALL*/ )
{
    return QJSValue();
}

/*QJSValue ExtensionImpl::SELECT_Point(double Xr, double Yr, double Zr, QString direction)
{
    return QJSValue();
}*/

QJSValue ExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point( QJSValue base_plane,
                                                                                  QJSValue base_axis,
                                                                                  QJSValue base_point )
{
    Q_UNUSED( base_plane );
    Q_UNUSED( base_axis );
    Q_UNUSED( base_point );

    return QJSValue();
}

QJSValue ExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_XY_dir( QJSValue point,
                                                                              QJSValue x_dir,
                                                                              QJSValue y_dir )
{
    Q_UNUSED( point );
    Q_UNUSED( x_dir );
    Q_UNUSED( y_dir );

    return QJSValue();
}

QJSValue ExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_YZ_dir( QJSValue point,
                                                                              QJSValue y_dir,
                                                                              QJSValue z_dir )
{
    Q_UNUSED( point );
    Q_UNUSED( y_dir );
    Q_UNUSED( z_dir );

    return QJSValue();
}

QJSValue ExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_XZ_dir( QJSValue point,
                                                                              QJSValue x_dir,
                                                                              QJSValue z_dir )
{
    Q_UNUSED( point );
    Q_UNUSED( x_dir );
    Q_UNUSED( z_dir );

    return QJSValue();
}


/*QJSValue ExtensionImpl::CONSTRAINTS_Create_3DReference_Point_on_curve(QJSValue curve, double t)
{
     return QJSValue();
}*/

QJSValue ExtensionImpl::CONSTRAINTS_Create_Base_CoorSys( QVariantList /*matr*/ )
{
    return QJSValue();
}

QJSValue ExtensionImpl::SOLID_Create_Block( QString /*name*/,
                                            double /*xOrigin*/,
                                            double /*yOrigin*/,
                                            double /*zOrigin*/,
                                            double /*xLen*/,
                                            double /*yLen*/,
                                            double /*zLen*/ )
{
    return QJSValue();
}

QJSValue ExtensionImpl::SOLID_Create_Protrusion_Extrude( QJSValue /*selected_sketch*/,
                                                         QString /*extrude_name*/,
                                                         double /*depth_start*/,
                                                         double /*depth_end*/,
                                                         bool /*sign*/ )
{
    return QJSValue();
}

void ExtensionImpl::SOLID_Create_Block_WDH( double /*x*/,
                                            double /*y*/,
                                            double /*z*/,
                                            double /*width*/,
                                            double /*depth*/,
                                            double /*height*/ )
{
}

void ExtensionImpl::SOLID_Create_Block_2Points( double /*x1*/,
                                                double /*y1*/,
                                                double /*z1*/,
                                                double /*x2*/,
                                                double /*y2*/,
                                                double /*z2*/ )
{
}

QJSValue ExtensionImpl::SKETCH_Create_2D_Line_2Points( QJSValue /*sketch*/, QString /*line_name*/,
                                                       double /*x1*/,
                                                       double /*y1*/,
                                                       double /*x2*/,
                                                       double /*y2*/,
                                                       bool /*flag*/ )
{
    return QJSValue();
}

QJSValue ExtensionImpl::SKETCH_Create_2D_Arc_3Points( QJSValue /*sketch*/, QString /*arc_name*/,
                                                      double /*x1*/,
                                                      double /*y1*/,
                                                      double /*x2*/,
                                                      double /*y2*/,
                                                      double /*x3*/,
                                                      double /*y3*/,
                                                      bool /*flag*/ )
{
    return QJSValue();
}

QJSValue ExtensionImpl::DUMP_All()
{
    return QJSValue();
}

QString ExtensionImpl::PreprocessModel()
{
    return "Preprocessed";
}

QString ExtensionImpl::Test()
{
    return "Test";
}

void ExtensionImpl::NamingTest()
{
    return;
}
