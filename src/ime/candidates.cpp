#include "candidates.h"

NAMESPACEBEGIN

bool LmaPsbItem::operator <(const LmaPsbItem &other) const
{
    // The real unified psb is psb1 / lma_len1 and psb2 * lma_len2
    // But we use psb1 * lma_len2 and psb2 * lma_len1 to get better
    // precision.
    const size_t up1 = psb * other.lma_len;
    const size_t up2 = other.psb * lma_len;
    return up1 <= up2;
}

Candidates::Itr Candidates::pull(int offs, int len) const
{
    if (Q_UNLIKELY(offs < 0)) offs += size();
    else if (Q_UNLIKELY(offs > size())) offs = size();
    if (Q_UNLIKELY(len < -1)) len = -1;
    ConstItr s = list.constBegin() + offs;
    ConstItr e = len == -1 || offs + len >= size()? list.constEnd(): s + len;
    return Itr(s - 1, e);
}

void Candidates::sortByPSB(int skip)
{
    qSort(list.begin() + skip, list.end());
}



NAMESPACEEND
