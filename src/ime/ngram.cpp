#include "ngram.h"
#include <QFile>

NAMESPACEBEGIN


bool NGram::load(QFile &fp)
{
    quint32 idx_num_;
    if (fp.read((char *)&idx_num_, 4) != 4) return false;
    lma_freq_idx_.resize(idx_num_);
    if (fp.read((char*)freq_codes_, sizeof (freq_codes_)) != sizeof (freq_codes_)) return false;
    if (fp.read((char*)lma_freq_idx_.data(), idx_num_) != idx_num_) return false;
    return fp.atEnd();
}



NAMESPACEEND
