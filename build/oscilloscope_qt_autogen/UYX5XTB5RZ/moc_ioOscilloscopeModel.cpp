/****************************************************************************
** Meta object code from reading C++ file 'ioOscilloscopeModel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ui/ioOscilloscopeModel.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ioOscilloscopeModel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN19ioOscilloscopeModel17OscilloscopeModelE_t {};
} // unnamed namespace

template <> constexpr inline auto ioOscilloscopeModel::OscilloscopeModel::qt_create_metaobjectdata<qt_meta_tag_ZN19ioOscilloscopeModel17OscilloscopeModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ioOscilloscopeModel::OscilloscopeModel",
        "samplesUpdated",
        "",
        "QList<double>",
        "normalizedSamples",
        "logLine",
        "text",
        "errorMessage",
        "startAcquisition",
        "stopAcquisition",
        "setScale",
        "value",
        "setShift",
        "setSampleHz",
        "hz",
        "setRawBufferSize",
        "bytes",
        "setReadDeviceIndex",
        "index",
        "setWriteDeviceIndex",
        "setDualFtdiOutput",
        "enabled",
        "setWriteBackToReadDevice",
        "setBlinkDb0",
        "deliverSample"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'samplesUpdated'
        QtMocHelpers::SignalData<void(const QVector<double> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'logLine'
        QtMocHelpers::SignalData<void(const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'errorMessage'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Slot 'startAcquisition'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'stopAcquisition'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setScale'
        QtMocHelpers::SlotData<void(double)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 11 },
        }}),
        // Slot 'setShift'
        QtMocHelpers::SlotData<void(double)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 11 },
        }}),
        // Slot 'setSampleHz'
        QtMocHelpers::SlotData<void(double)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 14 },
        }}),
        // Slot 'setRawBufferSize'
        QtMocHelpers::SlotData<void(int)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 16 },
        }}),
        // Slot 'setReadDeviceIndex'
        QtMocHelpers::SlotData<void(int)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Slot 'setWriteDeviceIndex'
        QtMocHelpers::SlotData<void(int)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Slot 'setDualFtdiOutput'
        QtMocHelpers::SlotData<void(bool)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'setWriteBackToReadDevice'
        QtMocHelpers::SlotData<void(bool)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'setBlinkDb0'
        QtMocHelpers::SlotData<void(bool)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'deliverSample'
        QtMocHelpers::SlotData<void(quint8)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::UChar, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<OscilloscopeModel, qt_meta_tag_ZN19ioOscilloscopeModel17OscilloscopeModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ioOscilloscopeModel::OscilloscopeModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19ioOscilloscopeModel17OscilloscopeModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19ioOscilloscopeModel17OscilloscopeModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN19ioOscilloscopeModel17OscilloscopeModelE_t>.metaTypes,
    nullptr
} };

void ioOscilloscopeModel::OscilloscopeModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<OscilloscopeModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->samplesUpdated((*reinterpret_cast<std::add_pointer_t<QList<double>>>(_a[1]))); break;
        case 1: _t->logLine((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->errorMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->startAcquisition(); break;
        case 4: _t->stopAcquisition(); break;
        case 5: _t->setScale((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 6: _t->setShift((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 7: _t->setSampleHz((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 8: _t->setRawBufferSize((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->setReadDeviceIndex((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->setWriteDeviceIndex((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->setDualFtdiOutput((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->setWriteBackToReadDevice((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 13: _t->setBlinkDb0((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 14: _t->deliverSample((*reinterpret_cast<std::add_pointer_t<quint8>>(_a[1]))); break;
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
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (OscilloscopeModel::*)(const QVector<double> & )>(_a, &OscilloscopeModel::samplesUpdated, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (OscilloscopeModel::*)(const QString & )>(_a, &OscilloscopeModel::logLine, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (OscilloscopeModel::*)(const QString & )>(_a, &OscilloscopeModel::errorMessage, 2))
            return;
    }
}

const QMetaObject *ioOscilloscopeModel::OscilloscopeModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ioOscilloscopeModel::OscilloscopeModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19ioOscilloscopeModel17OscilloscopeModelE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ioOscilloscopeModel::OscilloscopeModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void ioOscilloscopeModel::OscilloscopeModel::samplesUpdated(const QVector<double> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ioOscilloscopeModel::OscilloscopeModel::logLine(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ioOscilloscopeModel::OscilloscopeModel::errorMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
