#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <map>
#include <unordered_map>

#include "grid.h"
#include "gcanvas.h"
#include "huffman.h"
#include "queue.h"
#include "error.h"

using namespace std;

namespace {

static const int ZIGZAG[64] = {
    0,  1,  5,  6, 14, 15, 27, 28,
    2,  4,  7, 13, 16, 26, 29, 42,
    3,  8,  12, 17, 25, 30, 41, 43,
    9,  11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

inline uint16_t readBE16(istream& in) {
    uint8_t hi = 0, lo = 0;
    char chi, clo;
    if (!in.get(chi) || !in.get(clo)) {
        error("Unexpected end of file reading 16-bit value.");
    }
    hi = static_cast<uint8_t>(chi);
    lo = static_cast<uint8_t>(clo);
    return (uint16_t(hi) << 8) | uint16_t(lo);
}

inline int clampToByte(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

struct TmpNode {
    TmpNode* zero = nullptr;
    TmpNode* one  = nullptr;
    bool     isLeaf = false;
    uint8_t  symbol = 0;
};

void insertCanonicalCode(TmpNode*& root, uint32_t code, int length, uint8_t symbol) {
    if (!root) root = new TmpNode();
    TmpNode* node = root;
    for (int i = length - 1; i >= 0; --i) {
        int bit = (code >> i) & 1;
        if (bit == 0) {
            if (!node->zero) node->zero = new TmpNode();
            node = node->zero;
        } else {
            if (!node->one) node->one = new TmpNode();
            node = node->one;
        }
    }
    node->isLeaf = true;
    node->symbol = symbol;
}

EncodingTreeNode* convertToEncodingTree(TmpNode* t) {
    if (!t) return nullptr;
    if (t->isLeaf) {
        return new EncodingTreeNode(static_cast<char>(t->symbol));
    } else {
        EncodingTreeNode* left = convertToEncodingTree(t->zero);
        EncodingTreeNode* right = convertToEncodingTree(t->one);
        return new EncodingTreeNode(left, right);
    }
}

EncodingTreeNode* buildHuffmanTreeFromLengths(const array<uint8_t,16>& codeCounts,
                                              const vector<uint8_t>& symbols) {
    TmpNode* root = nullptr;
    uint32_t code = 0;
    int idx = 0;
    for (int len = 1; len <= 16; ++len) {
        code <<= 1;
        for (int i = 0; i < codeCounts[len - 1]; ++i) {
            if (idx >= (int)symbols.size()) {
                error("DHT: not enough symbols for given code lengths.");
            }
            insertCanonicalCode(root, code, len, symbols[idx++]);
            code++;
        }
    }
    EncodingTreeNode* encRoot = convertToEncodingTree(root);
    return encRoot;
}

struct QuantTable {
    uint16_t q[64] = {0};
};

struct HuffTable {
    EncodingTreeNode* root = nullptr;
};

struct Component {
    uint8_t id = 0;
    uint8_t h  = 1;
    uint8_t v  = 1;
    uint8_t tq = 0;
    uint8_t td_dc = 0;
    uint8_t ta_ac = 0;
    int     lastDC = 0;
};

struct Frame {
    uint8_t precision = 8;
    uint16_t width = 0;
    uint16_t height = 0;
    int numComponents = 0;
    vector<Component> comps;
    map<int, QuantTable> quantTables;
    map<int, HuffTable> dcTables;
    map<int, HuffTable> acTables;
    int restartInterval = 0;
};

struct IDCTTables {
    double cU[8][8];
    double cV[8][8];
    double alpha[8];
    IDCTTables() {
        const double PI = 3.14159265358979323846;
        for (int u = 0; u < 8; ++u) {
            alpha[u] = (u == 0) ? (1.0 / sqrt(2.0)) : 1.0;
            for (int x = 0; x < 8; ++x) {
                cU[u][x] = cos(((2.0 * x + 1.0) * u * PI) / 16.0);
            }
        }
        for (int v = 0; v < 8; ++v) {
            for (int y = 0; y < 8; ++y) {
                cV[v][y] = cos(((2.0 * y + 1.0) * v * PI) / 16.0);
            }
        }
    }
};

IDCTTables gIDCT;

void idct8x8(const int16_t in[64], uint8_t out[8][8]) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double sum = 0.0;
            for (int v = 0; v < 8; ++v) {
                for (int u = 0; u < 8; ++u) {
                    double Cu = gIDCT.alpha[u];
                    double Cv = gIDCT.alpha[v];
                    double coeff = static_cast<double>(in[v * 8 + u]);
                    sum += Cu * Cv * coeff * gIDCT.cU[u][x] * gIDCT.cV[v][y];
                }
            }
            int val = static_cast<int>(round(sum / 4.0) + 128.0);
            out[y][x] = static_cast<uint8_t>(clampToByte(val));
        }
    }
}

void parseDQT(istream& in, Frame& frame, uint16_t segLength) {
    int bytesRemaining = segLength - 2;
    while (bytesRemaining > 0) {
        uint8_t info = 0;
        if (!in.get(reinterpret_cast<char&>(info))) error("DQT: error reading info byte.");
        bytesRemaining--;
        int Pq = (info >> 4) & 0x0F;
        int Tq = info & 0x0F;
        if (Pq != 0 && Pq != 1) error("DQT: unsupported precision.");
        QuantTable qt;
        for (int i = 0; i < 64; ++i) {
            if (Pq == 0) {
                uint8_t v = 0;
                if (!in.get(reinterpret_cast<char&>(v))) error("DQT: error reading 8-bit q value.");
                bytesRemaining--;
                qt.q[ZIGZAG[i]] = v;
            } else {
                uint16_t v = readBE16(in);
                bytesRemaining -= 2;
                qt.q[ZIGZAG[i]] = v;
            }
        }
        frame.quantTables[Tq] = qt;
    }
}

void parseDHT(istream& in, Frame& frame, uint16_t segLength) {
    int bytesRemaining = segLength - 2;
    while (bytesRemaining > 0) {
        uint8_t info = 0;
        if (!in.get(reinterpret_cast<char&>(info))) error("DHT: error reading info byte.");
        bytesRemaining--;
        int Tc = (info >> 4) & 0x0F;
        int Th = info & 0x0F;
        array<uint8_t,16> codeCounts{};
        for (int i = 0; i < 16; ++i) {
            uint8_t cnt = 0;
            if (!in.get(reinterpret_cast<char&>(cnt))) error("DHT: error reading code count.");
            codeCounts[i] = cnt;
            bytesRemaining--;
        }
        int totalSymbols = 0;
        for (int i = 0; i < 16; ++i) totalSymbols += codeCounts[i];
        vector<uint8_t> symbols(totalSymbols);
        for (int i = 0; i < totalSymbols; ++i) {
            uint8_t s = 0;
            if (!in.get(reinterpret_cast<char&>(s))) error("DHT: error reading symbol.");
            symbols[i] = s;
            bytesRemaining--;
        }
        EncodingTreeNode* root = buildHuffmanTreeFromLengths(codeCounts, symbols);
        HuffTable ht;
        ht.root = root;
        if (Tc == 0) {
            frame.dcTables[Th] = ht;
        } else {
            frame.acTables[Th] = ht;
        }
    }
}

void parseSOF0(istream& in, Frame& frame, uint16_t /*segLength*/) {
    frame.precision = 0;
    if (!in.get(reinterpret_cast<char&>(frame.precision))) error("SOF0: error reading precision.");
    frame.height = readBE16(in);
    frame.width  = readBE16(in);
    uint8_t Nf = 0;
    if (!in.get(reinterpret_cast<char&>(Nf))) error("SOF0: error reading component count.");
    frame.numComponents = Nf;
    if (frame.numComponents != 1 && frame.numComponents != 3) {
        error("SOF0: only grayscale (1 comp) or YCbCr (3 comps) supported.");
    }
    frame.comps.resize(frame.numComponents);
    for (int i = 0; i < frame.numComponents; ++i) {
        uint8_t Ci=0, HV=0, Tq=0;
        if (!in.get(reinterpret_cast<char&>(Ci))) error("SOF0: error reading component id.");
        if (!in.get(reinterpret_cast<char&>(HV))) error("SOF0: error reading sampling factors.");
        if (!in.get(reinterpret_cast<char&>(Tq))) error("SOF0: error reading quant index.");
        frame.comps[i].id = Ci;
        frame.comps[i].h  = (HV >> 4) & 0x0F;
        frame.comps[i].v  = HV & 0x0F;
        frame.comps[i].tq = Tq;
        frame.comps[i].lastDC = 0;
        if (frame.comps[i].h < 1 || frame.comps[i].h > 4 ||
            frame.comps[i].v < 1 || frame.comps[i].v > 4) {
            error("Unsupported sampling factor (must be between 1 and 4).");
        }
    }
}

struct ScanHeader {
    int Ns = 0;
    vector<uint8_t> compIds;
    unordered_map<uint8_t, pair<uint8_t,uint8_t>> tableSelector;
    uint8_t Ss=0, Se=0, Ah=0, Al=0;
};

ScanHeader parseSOSHeader(istream& in, uint16_t segLength) {
    ScanHeader sh;
    uint8_t NsByte = 0;
    if (!in.get(reinterpret_cast<char&>(NsByte))) error("SOS: error reading Ns.");
    sh.Ns = NsByte;
    for (int i = 0; i < sh.Ns; ++i) {
        uint8_t cs = 0, tdta = 0;
        if (!in.get(reinterpret_cast<char&>(cs))) error("SOS: error reading Cs.");
        if (!in.get(reinterpret_cast<char&>(tdta))) error("SOS: error reading Td/Ta.");
        uint8_t Td = (tdta >> 4) & 0x0F;
        uint8_t Ta = tdta & 0x0F;
        sh.compIds.push_back(cs);
        sh.tableSelector[cs] = {Td, Ta};
    }
    if (!in.get(reinterpret_cast<char&>(sh.Ss))) error("SOS: error reading Ss.");
    if (!in.get(reinterpret_cast<char&>(sh.Se))) error("SOS: error reading Se.");
    uint8_t AhAl = 0;
    if (!in.get(reinterpret_cast<char&>(AhAl))) error("SOS: error reading Ah/Al.");
    sh.Ah = (AhAl >> 4) & 0x0F;
    sh.Al = AhAl & 0x0F;

    if (sh.Ss != 0 || sh.Se != 63 || sh.Ah != 0 || sh.Al != 0) {
        error("Only baseline sequential JPEG supported (Ss=0, Se=63, Ah=Al=0).");
    }

    int consumed = 1 + 2 * sh.Ns + 3;
    int extra = segLength - 2 - consumed;
    while (extra-- > 0) { char dummy; if (!in.get(dummy)) error("EOF in SOS segment."); }

    return sh;
}

void parseDRI(istream& in, Frame& frame, uint16_t segLength) {
    if (segLength != 4) {
        // Read but ignore malformed
        int toSkip = segLength - 2;
        while (toSkip-- > 0) { char dummy; if (!in.get(dummy)) error("EOF in DRI segment."); }
        return;
    }
    frame.restartInterval = readBE16(in);
}

class BitStream {
public:
    explicit BitStream(istream& in) : _in(in) {}

    bool readBit(int& outBit) {
        if (_bitIndex == 8) {
            if (!readNextByte()) return false;
        }
        outBit = (_bitBuffer >> (7 - _bitIndex)) & 1;
        _bitIndex++;
        return true;
    }

    void alignToByte() {
        _bitIndex = 8;
    }

    bool consumeRestartIfPresent() {
        if (_restartSeen) {
            _restartSeen = false;
            return true;
        }
        return false;
    }

    bool endOfScan() const { return _endOfScan; }
    uint8_t endMarker() const { return _endMarker; }

private:
    istream& _in;
    uint8_t _bitBuffer = 0;
    uint8_t _bitIndex  = 8;
    bool _restartSeen = false;
    bool _endOfScan = false;
    uint8_t _endMarker = 0;

    bool readNextByte() {
        char c;
        while (true) {
            if (!_in.get(c)) return false;
            uint8_t b = static_cast<uint8_t>(c);
            if (b == 0xFF) {
                char d;
                if (!_in.get(d)) return false;
                uint8_t nb = static_cast<uint8_t>(d);
                if (nb == 0x00) {
                    _bitBuffer = 0xFF;
                    _bitIndex = 0;
                    return true;
                } else if (nb >= 0xD0 && nb <= 0xD7) {
                    _restartSeen = true;
                    // skip marker and continue to next byte
                    continue;
                } else {
                    _endOfScan = true;
                    _endMarker = nb;
                    return false;
                }
            } else {
                _bitBuffer = b;
                _bitIndex = 0;
                return true;
            }
        }
    }
};

uint8_t decodeOneSymbolStream(EncodingTreeNode* tree, BitStream& bits) {
    EncodingTreeNode* node = tree;
    while (!node->isLeaf()) {
        int bit;
        if (!bits.readBit(bit)) error("Unexpected end of bits in Huffman decode.");
        node = (bit == 0) ? node->zero : node->one;
    }
    return static_cast<uint8_t>(node->getChar());
}

int readNBitsStream(BitStream& bits, int n) {
    int v = 0;
    for (int i = 0; i < n; ++i) {
        int bit;
        if (!bits.readBit(bit)) error("Unexpected end of bits while reading value bits.");
        v = (v << 1) | (bit & 1);
    }
    return v;
}

void decodeBlockNaturalStream(const Frame& frame,
                              Component& comp,
                              HuffTable& dcHuff,
                              HuffTable& acHuff,
                              BitStream& bits,
                              int16_t blockNat[64]) {
    int16_t zz[64];
    for (int i = 0; i < 64; ++i) zz[i] = 0;

    uint8_t cat = decodeOneSymbolStream(dcHuff.root, bits);
    int diffBits = readNBitsStream(bits, cat);
    int diff = (cat ? (diffBits >= (1 << (cat - 1)) ? diffBits : diffBits - ((1 << cat) - 1)) : 0);
    int DC = comp.lastDC + diff;
    comp.lastDC = DC;
    zz[0] = static_cast<int16_t>(DC);

    int k = 1;
    while (k < 64) {
        uint8_t sym = decodeOneSymbolStream(acHuff.root, bits);
        if (sym == 0x00) {
            break;
        } else if (sym == 0xF0) {
            k += 16;
            continue;
        } else {
            int run = (sym >> 4) & 0x0F;
            int size = sym & 0x0F;
            k += run;
            if (k >= 64) break;
            int valBits = readNBitsStream(bits, size);
            int val = (size ? (valBits >= (1 << (size - 1)) ? valBits : valBits - ((1 << size) - 1)) : 0);
            zz[k] = static_cast<int16_t>(val);
            k++;
        }
    }

    const QuantTable& qt = frame.quantTables.at(comp.tq);
    for (int i = 0; i < 64; ++i) {
        int natIdx = ZIGZAG[i];
        int16_t deq = static_cast<int16_t>(zz[i] * qt.q[natIdx]);
        blockNat[natIdx] = deq;
    }
}

struct Tile {
    uint8_t p[8][8];
};

inline uint8_t getTileSample(const vector<Tile>& tiles,
                             const Component& comp,
                             int col, int row) {
    if (col < 0) col = 0;
    if (row < 0) row = 0;
    int maxCol = comp.h * 8 - 1;
    int maxRow = comp.v * 8 - 1;
    if (col > maxCol) col = maxCol;
    if (row > maxRow) row = maxRow;
    int tileX = col / 8;
    int tileY = row / 8;
    int localX = col % 8;
    int localY = row % 8;
    int idx = tileY * comp.h + tileX;
    if (idx < 0 || idx >= (int)tiles.size()) return 0;
    return tiles[idx].p[localY][localX];
}

inline uint8_t sampleFromTilesBilinear(const vector<Tile>& tiles,
                                       const Component& comp,
                                       int Hmax, int Vmax,
                                       int mx, int my) {
    double u = ((mx + 0.5) * comp.h) / (double)Hmax - 0.5;
    double v = ((my + 0.5) * comp.v) / (double)Vmax - 0.5;
    int col0 = (int)floor(u);
    int row0 = (int)floor(v);
    int col1 = col0 + 1;
    int row1 = row0 + 1;
    double fx = u - col0;
    double fy = v - row0;

    uint8_t s00 = getTileSample(tiles, comp, col0, row0);
    uint8_t s10 = getTileSample(tiles, comp, col1, row0);
    uint8_t s01 = getTileSample(tiles, comp, col0, row1);
    uint8_t s11 = getTileSample(tiles, comp, col1, row1);

    double res = (1 - fx) * (1 - fy) * s00 +
                 fx       * (1 - fy) * s10 +
                 (1 - fx) * fy       * s01 +
                 fx       * fy       * s11;
    int r = (int)round(res);
    return (uint8_t)clampToByte(r);
}

inline void ycbcrToRgb(uint8_t Y, uint8_t Cb, uint8_t Cr, int& R, int& G, int& B) {
    double y = Y;
    double cb = Cb - 128.0;
    double cr = Cr - 128.0;
    int r = clampToByte(static_cast<int>(round(y + 1.402 * cr)));
    int g = clampToByte(static_cast<int>(round(y - 0.344136 * cb - 0.714136 * cr)));
    int b = clampToByte(static_cast<int>(round(y + 1.772 * cb)));
    R = r; G = g; B = b;
}

} // namespace

// Helper (ensure this is present once in your file)
int findComponentIndexById(const Frame& frame, uint8_t id) {
    for (int i = 0; i < (int)frame.comps.size(); ++i) {
        if (frame.comps[i].id == id) return i;
    }
    return -1;
}

void loadJpeg(GCanvas& img, string filename) {
    ifstream in(filename, ios::binary);
    if (!in) {
        error("Could not open JPEG file: " + filename);
    }

    uint8_t b1 = 0, b2 = 0;
    char c1, c2;
    if (!in.get(c1) || !in.get(c2)) {
        error("File too short to be a JPEG.");
    }
    b1 = static_cast<uint8_t>(c1);
    b2 = static_cast<uint8_t>(c2);
    if (b1 != 0xFF || b2 != 0xD8) {
        error("Not a JPEG file (missing SOI marker).");
    }

    Frame frame;
    bool gotSOF0 = false;
    Grid<int> grid;

    while (true) {
        uint8_t marker = 0;
        char c;
        do {
            if (!in.get(c)) {
                error("Unexpected end of file looking for JPEG marker.");
            }
        } while (static_cast<uint8_t>(c) != 0xFF);
        char mcode;
        if (!in.get(mcode)) error("EOF reading marker code.");
        marker = static_cast<uint8_t>(mcode);

        if (marker == 0xD9) {
            break;
        }

        if (marker == 0xDA) {
            uint16_t segLength = readBE16(in);
            ScanHeader sh = parseSOSHeader(in, segLength);

            vector<int> compIdxOrder;
            compIdxOrder.reserve(sh.compIds.size());
            for (uint8_t cid : sh.compIds) {
                int idx = findComponentIndexById(frame, cid);
                if (idx < 0) error("SOS references unknown component id.");
                frame.comps[idx].td_dc = sh.tableSelector[cid].first;
                frame.comps[idx].ta_ac = sh.tableSelector[cid].second;
                compIdxOrder.push_back(idx);
            }

            if (!gotSOF0) {
                error("SOS encountered before SOF0.");
            }

            BitStream bits(in);

            int width = frame.width;
            int height = frame.height;
            grid.resize(height, width);

            int Hmax = 1, Vmax = 1;
            for (const Component& comp : frame.comps) {
                Hmax = max(Hmax, (int)comp.h);
                Vmax = max(Vmax, (int)comp.v);
            }
            int mcuW = Hmax * 8;
            int mcuH = Vmax * 8;

            int mcusX = (width  + mcuW - 1) / mcuW;
            int mcusY = (height + mcuH - 1) / mcuH;

            auto getDC = [&](const Component& comp) -> HuffTable& {
                auto it = frame.dcTables.find(comp.td_dc);
                if (it == frame.dcTables.end()) error("Missing DC Huffman table.");
                return it->second;
            };
            auto getAC = [&](const Component& comp) -> HuffTable& {
                auto it = frame.acTables.find(comp.ta_ac);
                if (it == frame.acTables.end()) error("Missing AC Huffman table.");
                return it->second;
            };

            int idxY  = findComponentIndexById(frame, 1);
            int idxCb = findComponentIndexById(frame, 2);
            int idxCr = findComponentIndexById(frame, 3);
            if (frame.numComponents == 3) {
                if (idxY < 0 || idxCb < 0 || idxCr < 0) {
                    idxY = 0; idxCb = 1; idxCr = 2;
                }
            }

            int mcuCount = 0;
            for (int myMCU = 0; myMCU < mcusY; ++myMCU) {
                for (int mxMCU = 0; mxMCU < mcusX; ++mxMCU) {
                    vector<vector<Tile>> tiles(frame.numComponents);
                    for (int ord = 0; ord < (int)compIdxOrder.size(); ++ord) {
                        int ci = compIdxOrder[ord];
                        Component& comp = frame.comps[ci];
                        HuffTable& dcH = getDC(comp);
                        HuffTable& acH = getAC(comp);
                        for (int vy = 0; vy < comp.v; ++vy) {
                            for (int hx = 0; hx < comp.h; ++hx) {
                                int16_t block[64] = {0};
                                decodeBlockNaturalStream(frame, comp, dcH, acH, bits, block);
                                Tile t;
                                idct8x8(block, t.p);
                                tiles[ci].push_back(t);
                            }
                        }
                    }

                    for (int my = 0; my < mcuH; ++my) {
                        for (int mxp = 0; mxp < mcuW; ++mxp) {
                            int px = mxMCU * mcuW + mxp;
                            int py = myMCU * mcuH + my;
                            if (px >= width || py >= height) continue;

                            if (frame.numComponents == 1) {
                                const Component& Yc = frame.comps[0];
                                uint8_t Yval = sampleFromTilesBilinear(tiles[0], Yc, Hmax, Vmax, mxp, my);
                                int rgb = GCanvas::createRgbPixel(Yval, Yval, Yval);
                                grid[py][px] = rgb;
                            } else {
                                const Component& Yc  = frame.comps[idxY];
                                const Component& Cbc = frame.comps[idxCb];
                                const Component& Crc = frame.comps[idxCr];

                                uint8_t Yval  = sampleFromTilesBilinear(tiles[idxY],  Yc,  Hmax, Vmax, mxp, my);
                                uint8_t Cbval = sampleFromTilesBilinear(tiles[idxCb], Cbc, Hmax, Vmax, mxp, my);
                                uint8_t Crval = sampleFromTilesBilinear(tiles[idxCr], Crc, Hmax, Vmax, mxp, my);

                                int R, G, B;
                                ycbcrToRgb(Yval, Cbval, Crval, R, G, B);
                                int rgb = GCanvas::createRgbPixel(R, G, B);
                                grid[py][px] = rgb;
                            }
                        }
                    }

                    mcuCount++;
                    if (frame.restartInterval > 0 && (mcuCount % frame.restartInterval) == 0) {
                        bits.alignToByte();
                        if (bits.consumeRestartIfPresent()) {
                            for (auto& comp : frame.comps) comp.lastDC = 0;
                        }
                    }
                }
            }

            img.fromGrid(grid);
            return;
        } else {
            uint16_t segLength = readBE16(in);
            if (segLength < 2) error("Segment length too short.");

            switch (marker) {
            case 0xDB:
                parseDQT(in, frame, segLength);
                break;
            case 0xC4:
                parseDHT(in, frame, segLength);
                break;
            case 0xC0:
                parseSOF0(in, frame, segLength);
                gotSOF0 = true;
                {
                    int consumed = 8 + 3 * frame.numComponents;
                    int extra = segLength - 2 - consumed;
                    while (extra-- > 0) { char dummy; if (!in.get(dummy)) error("EOF in SOF0 segment."); }
                }
                break;
            case 0xDD:
                parseDRI(in, frame, segLength);
                break;
            default:
            {
                int toSkip = segLength - 2;
                while (toSkip-- > 0) {
                    char dummy;
                    if (!in.get(dummy)) error("EOF skipping marker segment.");
                }
            }
            break;
            }
        }
    }

    error("JPEG did not contain a decodable scan.");
}
