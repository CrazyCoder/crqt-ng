// coolreader-ng UI / Qt
// main.cpp - entry point

#include <qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif
#include "config.h"
#include "mainwindow.h"
#if QT_VERSION >= 0x050000
#include <QtCore/QTranslator>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDir>
#else
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#endif

#include <crengine.h>
#include <crtxtenc.h>

#if defined(QT_STATIC)
#if QT_VERSION >= 0x050000
#include <QtPlugin>
#if defined(Q_OS_UNIX)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#elif defined(Q_OS_WIN)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin);
#elif defined(Q_OS_MACOS)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif
#endif
#endif

// prototypes
void InitCREngineLog(const char* cfgfile);
bool InitCREngine(const char* exename, lString32Collection& fontDirs);
void ShutdownCREngine();
#if (USE_FREETYPE == 1)
bool getDirectoryFonts(lString32Collection& pathList, lString32Collection& ext, lString32Collection& fonts,
                       bool absPath);
#endif

static void printHelp() {
    printf("usage: crqt [options] [filename]\n"
           "Options:\n"
           "  -h or --help: this message\n"
           "  -v or --version: print program version\n"
           "  --loglevel=ERROR|WARN|INFO|DEBUG|TRACE: set logging level\n"
           "  --logfile=<filename>|stdout|stderr: set log file\n");
}

static void printVersion() {
    printf("cruiqt " VERSION "; crengine-ng-" CRE_NG_VERSION "\n");
}

int main(int argc, char* argv[]) {
    int res = 0;
    {
#ifdef DEBUG
        lString8 loglevel("TRACE");
        lString8 logfile("stdout");
#else
        lString8 loglevel("ERROR");
        lString8 logfile("stderr");
#endif
        int optpos = 1;
        for (int i = 1; i < argc; i++) {
            if (!strcmp("-h", argv[i]) || !strcmp("-?", argv[i]) || !strcmp("/?", argv[i]) ||
                !strcmp("--help", argv[i])) {
                printHelp();
                return 0;
            }
            if (!strcmp("-v", argv[i]) || !strcmp("/v", argv[i]) || !strcmp("--version", argv[i])) {
                printVersion();
                return 0;
            }
            if (!strcmp("--stats", argv[i]) && i < argc - 4) {
                if (i != argc - 5) {
                    printf("To calculate character encoding statistics, use cruiqt <infile.txt> <outfile.cpp> <codepagename> <langname>\n");
                    return 1;
                }
                lString8 list;
                FILE* out = fopen(argv[i + 2], "wb");
                if (!out) {
                    printf("Cannot create file %s", argv[i + 2]);
                    return 1;
                }
                MakeStatsForFile(argv[i + 1], argv[i + 3], argv[i + 4], 0, out, list);
                fclose(out);
                return 0;
            }
            lString8 s(argv[i]);
            if (s.startsWith(cs8("--loglevel="))) {
                loglevel = s.substr(11, s.length() - 11);
                optpos++;
            } else if (s.startsWith(cs8("--logfile="))) {
                logfile = s.substr(10, s.length() - 10);
                optpos++;
            }
        }

        // set logger
        if (logfile == "stdout")
            CRLog::setStdoutLogger();
        else if (logfile == "stderr")
            CRLog::setStderrLogger();
        else if (!logfile.empty())
            CRLog::setFileLogger(logfile.c_str());
        if (loglevel == "TRACE")
            CRLog::setLogLevel(CRLog::LL_TRACE);
        else if (loglevel == "DEBUG")
            CRLog::setLogLevel(CRLog::LL_DEBUG);
        else if (loglevel == "INFO")
            CRLog::setLogLevel(CRLog::LL_INFO);
        else if (loglevel == "WARN")
            CRLog::setLogLevel(CRLog::LL_WARN);
        else if (loglevel == "ERROR")
            CRLog::setLogLevel(CRLog::LL_ERROR);
        else
            CRLog::setLogLevel(CRLog::LL_FATAL);

        CRLog::info("main()");
        QApplication app(argc, argv);

        lString32 configDir32 = getHomeConfigDir();
        if (!LVDirectoryExists(configDir32))
            LVCreateDirectory(configDir32);

        lString32Collection fontDirs;
        lString32 homefonts32 = configDir32 + cs32("fonts");
        fontDirs.add(homefonts32);
#if MACOS == 1 && USE_FONTCONFIG != 1
        fontDirs.add(cs32("/Library/Fonts"));
        fontDirs.add(cs32("/System/Library/Fonts"));
        fontDirs.add(cs32("/System/Library/Fonts/Supplemental"));
        lString32 home = qt2cr(QDir::toNativeSeparators(QDir::homePath()));
        fontDirs.add(home + cs32("/Library/Fonts"));
#endif
        if (!InitCREngine(argv[0], fontDirs)) {
            printf("Cannot init CREngine - exiting\n");
            return 2;
        }
        if (argc >= 2 && !strcmp(argv[1], "unittest")) {
#if 0 && defined(_DEBUG)
			runTinyDomUnitTests();
#endif
            CRLog::info("UnitTests finished: exiting");
            ShutdownCREngine();
            return 0;
        }
        QString datadir = cr2qt(getMainDataDir());
        QString translations = datadir + "i18n/";
        QTranslator qtTranslator;
        if (qtTranslator.load("qt_" + QLocale::system().name(),
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                              QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
#else
                              QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
#endif
            QApplication::installTranslator(&qtTranslator);

        QTranslator myappTranslator;
        QString trname = "crqt_" + QLocale::system().name();
        CRLog::info("Using translation file %s from dir %s", UnicodeToUtf8(qt2cr(trname)).c_str(),
                    UnicodeToUtf8(qt2cr(translations)).c_str());
        if (myappTranslator.load(trname, translations))
            QApplication::installTranslator(&myappTranslator);
        else
            CRLog::error("Canot load translation file %s from dir %s", UnicodeToUtf8(qt2cr(trname)).c_str(),
                         UnicodeToUtf8(qt2cr(translations)).c_str());
        QString fileToOpen;
        for (int i = optpos; i < argc; i++) {
            lString8 opt(argv[i]);
            if (!opt.startsWith("--")) {
                if (fileToOpen.isEmpty())
                    fileToOpen = cr2qt(LocalToUnicode(opt));
            }
        }
        MainWindow w(fileToOpen);
        w.show();
        res = app.exec();
    }
    ShutdownCREngine();
    return res;
}

void ShutdownCREngine() {
    HyphMan::uninit();
    ShutdownFontManager();
    CRLog::setLogger(NULL);
}

#if (USE_FREETYPE == 1)
bool getDirectoryFonts(lString32Collection& pathList, lString32Collection& ext, lString32Collection& fonts,
                       bool absPath) {
    int foundCount = 0;
    lString32 path;
    for (int di = 0; di < pathList.length(); di++) {
        path = pathList[di];
        LVContainerRef dir = LVOpenDirectory(path.c_str());
        if (!dir.isNull()) {
            CRLog::trace("Checking directory %s", UnicodeToUtf8(path).c_str());
            for (int i = 0; i < dir->GetObjectCount(); i++) {
                const LVContainerItemInfo* item = dir->GetObjectInfo(i);
                lString32 fileName = item->GetName();
                //lString8 fn = UnicodeToLocal(fileName);
                //printf(" test(%s) ", fn.c_str() );
                if (!item->IsContainer()) {
                    bool found = false;
                    lString32 lc = fileName;
                    lc.lowercase();
                    for (int j = 0; j < ext.length(); j++) {
                        if (lc.endsWith(ext[j])) {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                        continue;
                    lString32 fn;
                    if (absPath) {
                        fn = path;
                        if (!fn.empty() && fn[fn.length() - 1] != PATH_SEPARATOR_CHAR)
                            fn << PATH_SEPARATOR_CHAR;
                    }
                    fn << fileName;
                    foundCount++;
                    fonts.add(fn);
                }
            }
        }
    }
    return foundCount > 0;
}
#endif

bool InitCREngine(const char* exename, lString32Collection& fontDirs) {
    CRLog::trace("InitCREngine(%s)", exename);
#if defined(_WIN32)
    lString32 appPath32 = getExeDir();
    lString32 cfgfile32 = appPath32 + "crui.ini";
    InitCREngineLog(UnicodeToUtf8(cfgfile32).c_str());
#endif
    InitFontManager(lString8::empty_str);
#if defined(_WIN32) && USE_FONTCONFIG != 1
    wchar_t sysdir_w[MAX_PATH + 1];
    GetWindowsDirectoryW(sysdir_w, MAX_PATH);
    lString32 fontdir = Utf16ToUnicode(sysdir_w);
    fontdir << "\\Fonts\\";
    lString8 fontdir8(UnicodeToUtf8(fontdir));
    // clang-format off
    const char* fontnames[] = {
        "arial.ttf",
        "ariali.ttf",
        "arialb.ttf",
        "arialbi.ttf",
        "arialn.ttf",
        "arialni.ttf",
        "arialnb.ttf",
        "arialnbi.ttf",
        "cour.ttf",
        "couri.ttf",
        "courbd.ttf",
        "courbi.ttf",
        "times.ttf",
        "timesi.ttf",
        "timesb.ttf",
        "timesbi.ttf",
        "comic.ttf",
        "comicbd.ttf",
        "verdana.ttf",
        "verdanai.ttf",
        "verdanab.ttf",
        "verdanaz.ttf",
        "bookos.ttf",
        "bookosi.ttf",
        "bookosb.ttf",
        "bookosbi.ttf",
        "calibri.ttf",
        "calibrii.ttf",
        "calibrib.ttf",
        "calibriz.ttf",
        "cambria.ttf",
        "cambriai.ttf",
        "cambriab.ttf",
        "cambriaz.ttf",
        "georgia.ttf",
        "georgiai.ttf",
        "georgiab.ttf",
        "georgiaz.ttf",
        NULL
    };
    // clang-format on
    for (int fi = 0; fontnames[fi]; fi++) {
        fontMan->RegisterFont(fontdir8 + fontnames[fi]);
    }
#endif // defined(_WIN32) && USE_FONTCONFIG!=1
    // Load font definitions into font manager
    // fonts are in files font1.lbf, font2.lbf, ... font32.lbf
    // use fontconfig

#if USE_FREETYPE == 1
    lString32Collection fontExt;
    fontExt.add(cs32(".ttf"));
    fontExt.add(cs32(".ttc"));
    fontExt.add(cs32(".otf"));
    fontExt.add(cs32(".pfa"));
    fontExt.add(cs32(".pfb"));

    lString32 datadir32 = getMainDataDir();
    lString32 fontDir32 = datadir32 + "fonts";
    fontDirs.add(fontDir32);
    LVAppendPathDelimiter(fontDir32);

    lString32Collection fonts;
    getDirectoryFonts(fontDirs, fontExt, fonts, true);

    // load fonts from file
    CRLog::debug("%d font files found", fonts.length());
    //if (!fontMan->GetFontCount()) {
    for (int fi = 0; fi < fonts.length(); fi++) {
        lString8 fn = UnicodeToLocal(fonts[fi]);
        CRLog::trace("loading font: %s", fn.c_str());
        if (!fontMan->RegisterFont(fn)) {
            CRLog::trace("    failed\n");
        }
    }
    //}
#endif // USE_FREETYPE==1

    if (!fontMan->GetFontCount()) {
        //error
#if (USE_FREETYPE == 1)
        printf("Fatal Error: Cannot open font file(s) .ttf \nCannot work without font\n");
#else
        printf("Fatal Error: Cannot open font file(s) font#.lbf \nCannot work without font\nUse FontConv utility to generate .lbf fonts from TTF\n");
#endif
        return false;
    }

    printf("%d fonts loaded.\n", fontMan->GetFontCount());

    return true;
}

void InitCREngineLog(const char* cfgfile) {
    if (!cfgfile) {
        CRLog::setStdoutLogger();
        CRLog::setLogLevel(CRLog::LL_TRACE);
        return;
    }
    lString32 logfname;
    lString32 loglevelstr =
#ifdef _DEBUG
            U"TRACE";
#else
            U"INFO";
#endif
    bool autoFlush = false;
    CRPropRef logprops = LVCreatePropsContainer();
    {
        LVStreamRef cfg = LVOpenFileStream(cfgfile, LVOM_READ);
        if (!cfg.isNull()) {
            logprops->loadFromStream(cfg.get());
            logfname = logprops->getStringDef(PROP_LOG_FILENAME, "stdout");
            loglevelstr = logprops->getStringDef(PROP_LOG_LEVEL, "TRACE");
            autoFlush = logprops->getBoolDef(PROP_LOG_AUTOFLUSH, false);
        }
    }
    CRLog::log_level level = CRLog::LL_INFO;
    if (loglevelstr == "OFF") {
        level = CRLog::LL_FATAL;
        logfname.clear();
    } else if (loglevelstr == "FATAL") {
        level = CRLog::LL_FATAL;
    } else if (loglevelstr == "ERROR") {
        level = CRLog::LL_ERROR;
    } else if (loglevelstr == "WARN") {
        level = CRLog::LL_WARN;
    } else if (loglevelstr == "INFO") {
        level = CRLog::LL_INFO;
    } else if (loglevelstr == "DEBUG") {
        level = CRLog::LL_DEBUG;
    } else if (loglevelstr == "TRACE") {
        level = CRLog::LL_TRACE;
    }
    if (!logfname.empty()) {
        if (logfname == "stdout")
            CRLog::setStdoutLogger();
        else if (logfname == "stderr")
            CRLog::setStderrLogger();
        else
            CRLog::setFileLogger(UnicodeToUtf8(logfname).c_str(), autoFlush);
    }
    CRLog::setLogLevel(level);
    CRLog::trace("Log initialization done.");
}
