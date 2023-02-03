#pragma once

#include "../extension/src/BaseExtension.h"
#include "../extension/src/IAbstractModeler.h"

class ExtensionImpl : public BaseExtension, public IAbstractModeler
{
    Q_OBJECT

private:
    ExtensionImpl();

public:
    inline static ExtensionImpl& Instance()
    {
        return m_instance;
    }

public:
    virtual void Initialize();

private:
    QWidget* GetTopWindow();
protected:
    QString PreprocessModel();
public slots:
    QJSValue DUMP_All();

public slots:
    void CheckContextIsValid();

protected:
    QString Test();
protected:
    void NamingTest();

public slots:
    QJSValue SKETCH_Open( QString skt_name, QJSValue csys );
public slots:
    QJSValue SKETCH_Close( QJSValue sketch );

    /*public slots: QJSValue SELECT_Face(double Xp, double Yp, double Zp,
                                           double Xr, double Yr, double Zr,
                                           int number);

    public slots: QJSValue SELECT_Edge(double Xp, double Yp, double Zp,
                                           double Xr, double Yr, double Zr,
                                           int number);*/

public slots:
    QJSValue SELECT_Object( double xPoint, double yPoint, double zPoint,
                            TopologyPriority topologyPriority );

    //public slots: QJSValue SELECT_Point(double Xr, double Yr, double Zr, QString direction);

public slots:
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point( QJSValue base_plane,
                                                                       QJSValue base_axis,
                                                                       QJSValue base_point );

public slots:
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_XY_dir( QJSValue point,
                                                                   QJSValue x_dir,
                                                                   QJSValue y_dir );

public slots:
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_YZ_dir( QJSValue point,
                                                                   QJSValue y_dir,
                                                                   QJSValue z_dir );

public slots:
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_XZ_dir( QJSValue point,
                                                                   QJSValue x_dir,
                                                                   QJSValue z_dir );

//    public slots: QJSValue CONSTRAINTS_Create_3DReference_Point_on_curve(QJSValue curve, double t);

public slots:
    QJSValue CONSTRAINTS_Create_Base_CoorSys( QVariantList matr );

public slots:
    QJSValue SOLID_Create_Block( QString name,
                                 double xOrigin,
                                 double yOrigin,
                                 double zOrigin,
                                 double xLen,
                                 double yLen,
                                 double zLen );

public slots:
    QJSValue SOLID_Create_Protrusion_Extrude( QJSValue selected_sketch,
                                              QString extrude_name,
                                              double depth_start,
                                              double depth_end,
                                              bool sign = true );

public slots:
    void SOLID_Create_Block_WDH( double x,
                                 double y,
                                 double z,
                                 double width,
                                 double depth,
                                 double height );

public slots:
    void SOLID_Create_Block_2Points( double x1,
                                     double y1,
                                     double z1,
                                     double x2,
                                     double y2,
                                     double z2 );

public slots:
    QJSValue SKETCH_Create_2D_Line_2Points( QJSValue sketch, QString line_name,
                                            double x1,
                                            double y1,
                                            double x2,
                                            double y2,
                                            bool flag );

public slots:
    QJSValue SKETCH_Create_2D_Arc_3Points( QJSValue sketch, QString arc_name,
                                           double x1,
                                           double y1,
                                           double x2,
                                           double y2,
                                           double x3,
                                           double y3,
                                           bool flag );

private:
    static ExtensionImpl m_instance;
};
