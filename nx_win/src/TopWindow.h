class TopWindow : public QWidget
{
    Q_OBJECT

public:
    TopWindow( HWND id );
public:
    ~TopWindow();

public:
    HWND parentWindow() const;

protected:
    void childEvent( QChildEvent* e );
protected:
    bool eventFilter( QObject* o, QEvent* e );

private:
    void saveFocus();
private:
    void resetFocus();

private:
    HWND hParent;
private:
    HWND prevFocus;
private:
    bool reenable_parent;
};
