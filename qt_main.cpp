#include "ioMainWindow.h"
#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <cstdlib>
#include <cstring>

namespace {

int parsePersonFromArgs(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--person") == 0 && i + 1 < argc) {
            const int v = std::atoi(argv[++i]);
            if (v == 1 || v == 2) {
                return v;
            }
        }
    }
    return 0;
}

/** Modal dialog: choose Person 1 or 2. Returns 1, 2, or 0 if cancelled. */
int askPersonNumber(QWidget *parent) {
    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Choose operator"));
    dialog.setModal(true);

    QVBoxLayout *root = new QVBoxLayout(&dialog);
    root->addWidget(new QLabel(
        QObject::tr("Which person is this UI for?\n"
                      "Run two instances at once by launching the program again and choosing Person 1 and Person 2.\n"
                      "Default read FTDI index: Person 1 → 0, Person 2 → 1 (change in the main window if needed).")));

    QHBoxLayout *row = new QHBoxLayout();
    QPushButton *b1 = new QPushButton(QObject::tr("Person 1"));
    QPushButton *b2 = new QPushButton(QObject::tr("Person 2"));
    QPushButton *cancel = new QPushButton(QObject::tr("Cancel"));
    row->addWidget(b1);
    row->addWidget(b2);
    row->addWidget(cancel);
    root->addLayout(row);

    int chosen = 0;
    QObject::connect(b1, &QPushButton::clicked, [&]() {
        chosen = 1;
        dialog.accept();
    });
    QObject::connect(b2, &QPushButton::clicked, [&]() {
        chosen = 2;
        dialog.accept();
    });
    QObject::connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return 0;
    }
    return chosen;
}

} // namespace

int main(int argc, char *argv[]) {
    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("OscilloscopeCS565"));
    QApplication::setOrganizationName(QStringLiteral("StevensCS565"));

    int person = parsePersonFromArgs(argc, argv);
    if (person != 1 && person != 2) {
        person = askPersonNumber(nullptr);
    }
    if (person != 1 && person != 2) {
        return 0;
    }

    ioMainWindow::MainWindow window(person);
    window.resize(960, 640);
    window.show();

    return application.exec();
}
