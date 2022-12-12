#ifndef PROJECTIONVIEW_H
#define PROJECTIONVIEW_H

#include <QWidget>

class ProjectionView : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectionView(QWidget *parent = nullptr);

private:
    void paintEvent(QPaintEvent* ) override;

};

#endif // PROJECTIONVIEW_H
