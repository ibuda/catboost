#include <library/resource/registry.h>
#include <util/stream/file.h>
#include <util/folder/path.h>

using namespace NResource;

class TAsmWriter {
private:
    IOutputStream& AsmOut;
    TString AsmPrefix;

public:
    TAsmWriter(IOutputStream& out, const TString& prefix) : AsmOut(out), AsmPrefix(prefix)
    {
    }

    void Write(TStringBuf filename, const TString& data, bool raw) {
        TString constname(Basename(filename));
        if (constname.rfind('.') != TStringBuf::npos) {
            constname = constname.substr(0, constname.rfind('.'));
        }
        WriteHeader(constname);
        if (raw) {
            WriteRaw(constname, data);
        } else {
            WriteIncBin(constname, filename, data);
        }
        WriteFooter(constname);
        WriteSymbolSize(constname);
    }

private:
    void WriteHeader(const TStringBuf& constname) {
        AsmOut << "global " << AsmPrefix << constname << "\n";
        AsmOut << "global " << AsmPrefix << constname << "Size\n";
        AsmOut << "SECTION .rodata\n";
    }

    void WriteFooter(TStringBuf constname) {
        AsmOut << AsmPrefix << constname << "Size:\n";
        AsmOut << "dd " << AsmPrefix << constname << ".end - " << AsmPrefix << constname << "\n";
    }

    void WriteSymbolSize(TStringBuf constname) {
        AsmOut << "%ifidn __OUTPUT_FORMAT__,elf64\n";
        AsmOut << "size " << AsmPrefix << constname << " " << AsmPrefix << constname << ".end - " << AsmPrefix << constname << "\n";
        AsmOut << "size " << AsmPrefix << constname << "Size 4\n";
        AsmOut << "%endif\n";
    }

    void WriteIncBin(TStringBuf constname, TStringBuf filename, const TString& data) {
        AsmOut << AsmPrefix << constname << ":\nincbin \"" << Basename(filename) << "\"\n";
        AsmOut << ".end:\n";
        TFixedBufferFileOutput out(filename.data());
        out << data;
    }

    void WriteRaw(TStringBuf constname, const TString& data) {
        AsmOut << AsmPrefix << constname << ":\ndb ";
        for (size_t i = 0; i < data.size() - 1; i++) {
            unsigned char c = static_cast<unsigned char>(data[i]);
            AsmOut << IntToString<10, unsigned char>(c) << ",";
        }
        AsmOut << IntToString<10, unsigned char>(static_cast<unsigned char>(data[data.size() - 1])) << "\n";
        AsmOut << ".end:\n";
    }

    TString Basename(TStringBuf origin) {
        TString result(origin);
        if (result.rfind('/') != TString::npos) {
            result = result.substr(result.rfind('/') + 1);
        } else if (result.rfind('\\') != TString::npos) {
            result = result.substr(result.rfind('\\') + 1);
        }
        return result;
    }
};

static TString CompressPath(const TVector<TStringBuf>& replacements, TStringBuf in) {
    for (auto r : replacements) {
        TStringBuf from, to;
        r.Split('=', from, to);
        if (in.StartsWith(from)) {
            return Compress(TString(to) + in.SubStr(from.Size()));
        }
    }

    return Compress(in);
}

int main(int argc, char** argv) {
    if (argc < 4) {
        Cerr << "usage: " << argv[0] << "asm_output --prefix? [-? origin_resource ro_resource]+" << Endl;

        return 1;
    }

    TVector<TStringBuf> replacements;

    argv++;
    TFixedBufferFileOutput asmout(*argv);
    argv++;
    TString prefix;
    if (TStringBuf(*argv) == "--prefix") {
        prefix = "_";
        argv++;
    }
    else {
        prefix = "";
    }

    while (TStringBuf(*argv).StartsWith("--replace=")) {
        replacements.push_back(TStringBuf(*argv).SubStr(AsStringBuf("--replace=").Size()));
        argv++;
    }

    TAsmWriter aw(asmout, prefix);
    bool raw;
    while (*argv) {
        TString compressed;
        if (AsStringBuf("-") == *argv) {
            argv++;
            compressed = CompressPath(replacements, TStringBuf(*argv));
            raw = true;
        }
        else {
            TUnbufferedFileInput inp(*argv);
            TString data = inp.ReadAll();
            compressed = Compress(TStringBuf(data.data(), data.size()));
            raw = false;
        }
        argv++;
        aw.Write(*argv, compressed, raw);
        argv++;
    }
    return 0;
}
