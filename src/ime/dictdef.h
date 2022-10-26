#ifndef DICTDEF
#define DICTDEF

#define NAMESPACEBEGIN namespace IME {
#define NAMESPACEEND }
#define pNull NULL


#define kHalfSpellingIdNum 29
#define kFullSplIdStart (kHalfSpellingIdNum + 1)
#define kValidSplCharNum   26

#define kMaxLemmaSize 8
#define kCodeBookSize 256
 // Actually, a Id occupies 3 bytes in storage.
#define kLemmaIdSize 3

// The maximum buffer to store LmaPsbItems.
#define kMaxLmaPsbItems 1450

#define kMaxSearchSteps 40
#define kMaxRowNum kMaxSearchSteps

#endif // DICTDEF

