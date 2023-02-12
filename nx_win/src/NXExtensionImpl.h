#pragma once

#include "../../extension/src/BaseExtension.h"

/* This defines the UF_CALL(X) macro. */
#define UF_CALL(X) (NXExtensionImpl::Report( __FILE__, __LINE__, #X, (X)))

class NXExtensionImpl : public BaseExtension
{
    Q_OBJECT

private:
    NXExtensionImpl();
public:
    ~NXExtensionImpl();

public:
    inline static NXExtensionImpl& Instance()
    {
        return m_instance;
    }

public:
    virtual void Initialize();
public:
    virtual void Terminate();

    //------------------------- Callback Prototypes -----------------------
private:
    static UF_MB_cb_status_t RunEditorAction( UF_MB_widget_t widget,
                                              UF_MB_data_t client_data,
                                              UF_MB_activated_button_p_t call_button );

    static LRESULT CALLBACK QtFilterProc( int nCode,
                                          WPARAM wParam,
                                          LPARAM lParam );

    QWidget* GetTopWindow();

    void InitQt();

//protected:
//    QString PreprocessModel();
protected:
    Variant ExtractVariant();
//public slots:
//    void CheckContextIsValid();
//public slots:
//    QJSValue DUMP_All();

//protected:
//    QString Test();
//protected:
//    void NamingTest();
    // for extrude
public:
    uf_list_p_t objectList;
//public:
//    bool firstSolidFlag;
public:
    tag_t globalTagPart;
public:
    QVector<int> getAllFace();
public:
    QVector<int> getAllEdge();

    //public: QVector<int> allFace;
    //public: QVector<int> allEdge;
    //public: QVector<int> allSoPoint;

    /*public: struct intersect_info_face
    {
    int intersected_face;
    double t;
    };

    public: struct intersect_info_edge
    {
    int intersected_edge;
    double t;
    };

    public:    QVector<intersect_info_face> allIntersectFace;
    public:    QVector<intersect_info_edge> allIntersectEdge;*/

    // other function
public:
    void MapToWorld( double sket_csys[12], double point[3] ); // translate coordinats
public:
    static int Report( const char* file, int line, const char* call, int irc );

private:
    int m_registered;

private:
    static NXExtensionImpl m_instance;
private:
    static HHOOK m_hHook;
private:
    static UF_MB_action_t m_actionTable[];
};
