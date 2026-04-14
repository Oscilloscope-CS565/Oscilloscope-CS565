#ifndef IO_ABSTRACT_OSCILLOSCOPE_VIEW_H
#define IO_ABSTRACT_OSCILLOSCOPE_VIEW_H

#include <QString>
#include <QWidget>

namespace ioOscilloscopeModel {
class OscilloscopeModel;
}

namespace ioAbstractOscilloscopeView {

class AbstractOscilloscopeView {
public:
    virtual ~AbstractOscilloscopeView() = default;

    virtual QWidget *asWidget() = 0;
    virtual QString viewTitle() const = 0;

    virtual void bindModel(ioOscilloscopeModel::OscilloscopeModel *model) = 0;
    virtual void unbindModel() = 0;
};

} // namespace ioAbstractOscilloscopeView

#endif
