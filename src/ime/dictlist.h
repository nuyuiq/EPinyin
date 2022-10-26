#ifndef DICTLIST_H
#define DICTLIST_H

#include "dictdef.h"
#include <QVector>
class QFile;

NAMESPACEBEGIN

struct SpellingId {
    quint16 half_splid:5;
    quint16 full_splid:11;
};

struct DictList
{
    // Number of SingCharItem. The first is blank, because id 0 is invalid.
    quint32 scis_num_;
    QVector<quint16> scis_hz_;
    QVector<SpellingId> scis_splid_;
    // The large memory block to store the word list.
    QVector<quint16> buf_;
    // Starting position of those words whose lengths are i+1, counted in
    // char16
    quint32 start_pos_[kMaxLemmaSize + 1];
    quint32 start_id_[kMaxLemmaSize + 1];

    bool load(QFile &fp);
    // Get the hanzi string for the given id
    QString getLemmaStr(quint32 id) const;

};

NAMESPACEEND

#endif // DICTLIST_H
