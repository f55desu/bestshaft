#pragma once

class IAbstractModeler
{
public:
    enum TopologyPriority
    {
        TP_ALL = 0,
        TP_VERTEX = 1,
        TP_EDGE = 2,
        TP_FACE = 3
    };

public:
    virtual void CheckContextIsValid() = 0;

public:
    virtual QJSValue DUMP_All() = 0;

public:
    virtual QJSValue SKETCH_Open( QString skt_name, QJSValue csys ) = 0;

public:
    virtual QJSValue SKETCH_Close( QJSValue sketch ) = 0;

public:
    virtual QJSValue SKETCH_Create_2D_Line_2Points( QJSValue sketch,
                                                    QString line_name,
                                                    double x1,
                                                    double y1,
                                                    double x2,
                                                    double y2,
                                                    bool flag = true ) = 0;
public:
    virtual QJSValue SKETCH_Create_2D_Arc_3Points( QJSValue sketch, QString arc_name,
                                                   double x1,
                                                   double y1,
                                                   double x2,
                                                   double y2,
                                                   double x3,
                                                   double y3,
                                                   bool flag = true ) = 0;

    /*public: virtual QJSValue SELECT_Face(double Xp, double Yp, double Zp,
                                             double Xr, double Yr, double Zr,
                                             int number) = 0;

    public: virtual QJSValue SELECT_Edge(double Xp, double Yp, double Zp,
                                             double Xr, double Yr, double Zr,
                                             int number) = 0;*/

    //public: virtual QJSValue SELECT_Point(double Xr, double Yr, double Zr, QString direction) = 0;

public:
    virtual QJSValue SELECT_Object( double xPoint, double yPoint, double zPoint,
                                    TopologyPriority topologyPriority = TP_ALL ) = 0;

public:
    virtual QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point( QJSValue base_plane,
                                                                               QJSValue base_axis,
                                                                               QJSValue base_point ) = 0;

public:
    virtual QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_XY_dir( QJSValue point,
                                                                           QJSValue x_dir,
                                                                           QJSValue y_dir ) = 0;

public:
    virtual QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_YZ_dir( QJSValue point,
                                                                           QJSValue y_dir,
                                                                           QJSValue z_dir ) = 0;

public:
    virtual QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_XZ_dir( QJSValue point,
                                                                           QJSValue x_dir,
                                                                           QJSValue z_dir ) = 0;

//public: virtual QJSValue CONSTRAINTS_Create_3DReference_Point_on_curve(QJSValue curve, double t) = 0;
public:
    virtual QJSValue CONSTRAINTS_Create_Base_CoorSys( QVariantList matr ) = 0;

public:
    virtual QJSValue SOLID_Create_Block( QString name,
                                         double xOrigin,
                                         double yOrigin,
                                         double zOrigin,
                                         double xLen,
                                         double yLen,
                                         double zLen ) = 0;

public:
    virtual QJSValue SOLID_Create_Protrusion_Extrude( QJSValue selected_sketch,
                                                      QString extrude_name,
                                                      double depth_start,
                                                      double depth_end,
                                                      bool sign = true ) = 0;

public:
    virtual void SOLID_Create_Block_WDH( double x,
                                         double y,
                                         double z,
                                         double width,
                                         double depth,
                                         double height ) = 0;

public:
    virtual void SOLID_Create_Block_2Points( double x1,
                                             double y1,
                                             double z1,
                                             double x2,
                                             double y2,
                                             double z2 ) = 0;
};
