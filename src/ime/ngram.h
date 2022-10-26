#ifndef NGRAM_H
#define NGRAM_H

#include "dictdef.h"
#include <QVector>
class QFile;

NAMESPACEBEGIN

typedef quint8 CODEBOOK_TYPE;
typedef quint16 LmaScoreType;

class NGram
{
    LmaScoreType freq_codes_[kCodeBookSize];
    QVector<CODEBOOK_TYPE> lma_freq_idx_;

public:
    bool load(QFile &fp);
    inline LmaScoreType getUniPSB(quint32 lmaId) const;
};

LmaScoreType NGram::getUniPSB(quint32 lmaId) const
{
    Q_ASSERT(int (lmaId) < lma_freq_idx_.size());
    return freq_codes_[lma_freq_idx_[lmaId]];
}

NAMESPACEEND

#endif // NGRAM_H
