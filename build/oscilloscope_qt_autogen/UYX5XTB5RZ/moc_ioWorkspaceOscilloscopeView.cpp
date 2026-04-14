/****************************************************************************
** Meta object code from reading C++ file 'ioWorkspaceOscilloscopeView.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ui/ioWorkspaceOscilloscopeView.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ioWorkspaceOscilloscopeView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN27ioWorkspaceOscilloscopeView25WorkspaceOscilloscopeViewE_t {};
} // unnamed namespace

template <> constexpr inline auto ioWorkspaceOscilloscopeView::WorkspaceOscilloscopeView::qt_create_metaobjectdata<qt_meta_tag_ZN27ioWorkspaceOscilloscopeView25WorkspaceOscilloscopeViewE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ioWorkspaceOscilloscopeView::WorkspaceOscilloscopeView",
        "onSamplesUpdated",
        "",
        "QList<double>",
        "samples",
        "onLogLine",
        "text",
        "handleStart",
        "handleStop",
        "applyScale",
        "value",
        "applyShift",
        "applySampleHz",
        "applyBufferSize",
        "applyReadIndex",
        "applyWriteIndex",
        "applyDualFtdi",
        "Qt::CheckState",
        "state",
        "applyWriteBack",
        "applyBlinkDb0",
        "refreshWriteBackAvailability"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onSamplesUpdated'
        QtMocHelpers::SlotData<void(const QVector<double> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Slot 'onLogLine'
        QtMocHelpers::SlotData<void(const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Slot 'handleStart'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleStop'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'applyScale'
        QtMocHelpers::SlotData<void(double)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 10 },
        }}),
        // Slot 'applyShift'
        QtMocHelpers::SlotData<void(double)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 10 },
        }}),
        // Slot 'applySampleHz'
        QtMocHelpers::SlotData<void(double)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 10 },
        }}),
        // Slot 'applyBufferSize'
        QtMocHelpers::SlotData<void(int)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 10 },
        }}),
        // Slot 'applyReadIndex'
        QtMocHelpers::SlotData<void(int)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 10 },
        }}),
        // Slot 'applyWriteIndex'
        QtMocHelpers::SlotData<void(int)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 10 },
        }}),
        // Slot 'applyDualFtdi'
        QtMocHelpers::SlotData<void(Qt::CheckState)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 17, 18 },
        }}),
        // Slot 'applyWriteBack'
        QtMocHelpers::SlotData<void(Qt::CheckState)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 17, 18 },
        }}),
        // Slot 'applyBlinkDb0'
        QtMocHelpers::SlotData<void(Qt::CheckState)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 17, 18 },
        }}),
        // Slot 'refreshWriteBackAvailability'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WorkspaceOscilloscopeView, qt_meta_tag_ZN27ioWorkspaceOscilloscopeView25WorkspaceOscilloscopeViewE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ioWorkspaceOscilloscopeView::WorkspaceOscilloscopeView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN27ioWorkspaceOscilloscopeView25WorkspaceOscilloscopeViewE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN27ioWorkspaceOscilloscopeView25WorkspaceOscilloscopeViewE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN27ioWorkspaceOscilloscopeView25WorkspaceOscilloscopeViewE_t>.metaTypes,
    nullptr
} };

void ioWorkspaceOscilloscopeView::WorkspaceOscilloscopeView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WorkspaceOscilloscopeView *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onSamplesUpdated((*reinterpret_cast<std::add_pointer_t<QList<double>>>(_a[1]))); break;
        case 1: _t->onLogLine((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->handleStart(); break;
        case 3: _t->handleStop(); break;
        case 4: _t->applyScale((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 5: _t->applyShift((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 6: _t->applySampleHz((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 7: _t->applyBufferSize((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->applyReadIndex((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->applyWriteIndex((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->applyDualFtdi((*reinterpret_cast<std::add_pointer_t<Qt::CheckState>>(_a[1]))); break;
        case 11: _t->applyWriteBack((*reinterpret_cast<std::add_pointer_t<Qt::CheckState>>(_a[1]))); break;
        case 12: _t->applyBlinkDb0((*reinterpret_cast<std::add_pointer_t<Qt::CheckState>>(_a[1]))); break;
        case 13: _t->refreshWriteBackAvailability(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<double> >(); break;
            }
            break;
        }
    }
}

const QMetaObject *ioWorkspaceOscilloscopeView::WorkspaceOscilloscopeView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ioWorkspaceOscilloscopeView::WorkspaceOscilloscopeView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN27ioWorkspaceOscilloscopeView25WorkspaceOscilloscopeViewE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "ioAbstractOscilloscopeView::AbstractOscilloscopeView"))
        return static_cast< ioAbstractOscilloscopeView::AbstractOscilloscopeView*>(this);
    return QWidget::qt_metacast(_clname);
}

int ioWorkspaceOscilloscopeView::WorkspaceOscilloscopeView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
