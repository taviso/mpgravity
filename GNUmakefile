CC=cl.exe
RC=rc.exe
MIDL=midl.exe
MSBUILD=msbuild.exe
CMAKE=cmake.exe
CPPFLAGS=/I include /D NDEBUG /D WIN32 /D MICROPLANET /D MP_PACKBYTE /D _USE_32BIT_TIME_T
RFLAGS=/nologo
CFLAGS=/nologo $(CPPFLAGS) /Zi /O2 /guard:cf /FS
LFLAGS=/nologo /machine:x86
VFLAGS=-no_logo
MFLAGS=/p:Configuration=Release /nologo /m /v:q
MIDLFLAGS=
CXXFLAGS=/nologo $(CPPFLAGS) /Zi /O2 /EHsc /GA /guard:cf /FS
LDLIBS=comctl32 winmm version
LDFLAGS=/MD
LINKFLAGS=/ignore:4099 /subsystem:windows
VSDEVCMD=cmd.exe /c vsdevcmd.bat

.PHONY: clean

all: mpgravity.exe

Sources=advmhdr.cpp advphdr.cpp artact.cpp artbank.cpp artclean.cpp            \
        artdata.cpp article.cpp article2.cpp articlei.cpp artnode.cpp          \
        artrefs.cpp arttext.cpp autodrw.cpp autofont.cpp autoprio.cpp          \
        basfview.cpp btnlock.cpp codepg.cpp custsig.cpp custview.cpp           \
        dialman.cpp dlgappnd.cpp DlgQueryMsgID.cpp DlgQuickFilter.cpp          \
        evtdlg.cpp evtdtl.cpp evtlog.cpp expirable.cpp expire.cpp fetchart.cpp \
        fltbar.cpp fontdlg.cpp friched.cpp gallery.cpp globals.cpp             \
        GravityNewsgroup.cpp grpdb.cpp hd.cpp hierlbx.cpp hourglas.cpp         \
        Humble.cpp idxlst.cpp intvec.cpp ipcgal.cpp iterhn.cpp killprfx.cpp    \
        lay3lbx.cpp laylbx.cpp laytlbx.cpp lbxutil.cpp licutil.cpp             \
        LimitHdr.cpp  log.cpp mailadr.cpp mainfrm.cpp mdichild.cpp memshak.cpp \
        memspot.cpp mimatd.cpp mimedlg.cpp mlayout.cpp mpexcept.cpp            \
        mpexdisp.cpp mppath.cpp mprange.cpp mprelset.cpp mpserial.cpp          \
        msgid.cpp mywords.cpp nameutil.cpp navigdef.cpp ncmdline.cpp news.cpp  \
        newsdb.cpp newsdoc.cpp newsfeed.cpp newsgrp.cpp newsopt.cpp            \
        newspump.cpp newsrc.cpp newssock.cpp newsurl.cpp ngdlg.cpp nglib.cpp   \
        nglist.cpp ngstat.cpp ngutil.cpp noselebx.cpp nsigdlg.cpp odlist.cpp   \
        online.cpp outbox.cpp outbxdlg.cpp Picksvr.cpp Picture.cpp             \
        pikngdlg.cpp pktxtclr.cpp PngImage.cpp pobject2.cpp postpfrm.cpp       \
        primes.cpp printing.cpp pumpjob.cpp qprint.cpp rasdlg.cpp readavc.cpp  \
        regex.cpp rgbase.cpp rgbkgrnd.cpp rgcomp.cpp rgdir.cpp rgfont.cpp      \
        rglaymdi.cpp rglaywn.cpp rgmgr.cpp rgpurg.cpp rgserv.cpp rgstor.cpp    \
        rgswit.cpp rgsys.cpp rgui.cpp rgurl.cpp rgwarn.cpp rgxcopy.cpp         \
        rtfspt.cpp rxsearch.cpp safelnch.cpp SECURITY.cpp selrvw.cpp           \
        servchng.cpp servcp.cpp server.cpp servopts.cpp servpick.cpp           \
        servrang.cpp sharesem.cpp sigcomb.cpp smtp.cpp socktrak.cpp            \
        sortart.cpp spell.cpp SplashScreenEx.cpp splitter.cpp srchedvw.cpp     \
        srchfrm.cpp srchlvw.cpp ssutil.cpp statbar.cpp statchg.cpp             \
        statsrch.cpp statvec.cpp stdafx.cpp strext.cpp sysclr.cpp tabedit.cpp  \
        taglist.cpp taskcomp.cpp taskcon.cpp tasker.cpp taskfol.cpp            \
        tbozobin.cpp TCharCoding.cpp tcompdlg.cpp tdecdlg.cpp tdecjob.cpp      \
        tdecq.cpp tdecthrd.cpp tdecutil.cpp TDraftsDlg.cpp terrdlg.cpp         \
        tglobopt.cpp thrdact.cpp thread.cpp thredlst.cpp thredpl.cpp           \
        tidarray.cpp timek.cpp timemb.cpp timeutil.cpp timpword.cpp            \
        titlelbx.cpp titlevw.cpp tmandec.cpp tmanrule.cpp tmpllbx.cpp          \
        tmrbar.cpp tmsgbx.cpp tmutex.cpp tnews3md.cpp topenurl.cpp topexcp.cpp \
        tpendec.cpp tpostbtn.cpp tprndlg.cpp tprnjob.cpp tprnq.cpp             \
        tprnthrd.cpp TransparentBitmapButton.cpp tras.cpp TrcDlg.cpp           \
        tregdlg.cpp trendlg.cpp triched.cpp trxdlg.cpp tscoring.cpp            \
        tscribe.cpp tsearch.cpp TSpellDlg.cpp tsrchdlg.cpp tstrlist.cpp        \
        tsubjct.cpp tsubscri.cpp turldde.cpp turldef.cpp tutldlg.cpp           \
        tutljob.cpp tutlq.cpp tutlthrd.cpp txlstdlg.cpp uimem.cpp uipipe.cpp   \
        urlsppt.cpp usrdisp.cpp utilerr.cpp utilpump.cpp utilrout.cpp          \
        utilsize.cpp utilstr.cpp vcrdlg.cpp vcrrun.cpp veraddr.cpp vfiltad.cpp \
        vfilter.cpp vfltdlg.cpp vlist.cpp vlrare.cpp vlscroll.cpp vlsetcur.cpp \
        warndlg.cpp wndstat.cpp

HelperClasses=8859x.cpp bits.cpp boyerc.cpp bucket.cpp comlbx.cpp critsect.cpp \
        dbutil.cpp dirpick.cpp dllver.cpp fileutil.cpp gdiutil.cpp genutil.cpp \
        ndirpick.cpp regutil.cpp StdioFileEx.cpp

ArticleView=artdisp.cpp artview.cpp attdoc.cpp TXFaceWnd.cpp

ThreadView=thrdlvw.cpp thrdlvwx.cpp treehctl.cpp

GroupView=grplist.cpp newsview.cpp

ComposeDlg=attinfo.cpp attview.cpp compfrm.cpp comptool.cpp compview.cpp       \
        postbody.cpp postdoc.cpp posthdr.cpp postmdi.cpp posttool.cpp

MainWindow=DkToolBar.cpp

OptionsSetup=decodpg.cpp dialuppg.cpp encodpg.cpp fontpg.cpp gstorpg.cpp       \
        kidsecch.cpp kidsecgt.cpp kidsecst.cpp kidsecur.cpp layoutpg.cpp       \
        navigpg.cpp newsrcpg.cpp ngfltpg.cpp nggenpg.cpp ngoverpg.cpp          \
        ngpurg.cpp optshet.cpp replypg.cpp servpost.cpp srvoptg.cpp            \
        tconnpg.cpp tdisppg.cpp tdlgtmpv.cpp thotlink.cpp toperpg.cpp          \
        TOptionsMark.cpp tpostpg.cpp tprefpg.cpp truleact.cpp trulecon.cpp     \
        trulegen.cpp TServersPage.cpp tsetpg.cpp TSigPage.cpp tsigpg.cpp       \
        tspellpg.cpp tthredpg.cpp twarnpg.cpp twastepg.cpp update.cpp
Rules=artrule.cpp rules.cpp rulesdlg.cpp rulestat.cpp ruleutil.cpp
Filters=
OutboxDlg=
DraftDlg=
Binaries=coding.cpp crcnotab.cpp crypt.cpp decoding.cpp

Hunspell=affentry.cxx affixmgr.cxx csutil.cxx dictmgr.cxx filemgr.cxx          \
         hashmgr.cxx hunspell.cxx hunzip.cxx phonet.cxx replist.cxx            \
         suggestmgr.cxx utf_info.cxx
Hunspell:=$(addprefix hunspell-1.2.8/src/hunspell/,$(Hunspell))

Png=png.c pngerror.c pngget.c pngmem.c pngpread.c pngread.c pngrio.c           \
        pngrtran.c pngrutil.c pngset.c pngtrans.c pngvcrd.c pngwio.c           \
        pngwrite.c pngwtran.c pngwutil.c
Png:=$(addprefix png/code/,$(Png))

Pcre=chartables.c get.c maketables.c pcre.c study.c
Pcre:=$(addprefix pcre/,$(Pcre))

Winface=facebmp.c
Winface:=$(addprefix libwinface/,$(Winface))

Zlib=adler32.c compress.c crc32.c deflate.c gzio.c infback.c inflate.c         \
        inffast.c inftrees.c trees.c uncompr.c zutil.c
Zlib:=$(addprefix zlib/code/,$(Zlib))

Compface=arith.c compface.c compress.c file.c gen.c uncompface.c
Compface:=$(addprefix compface/,$(Compface))

%_i.h: %.odl
	$(MIDL) $(MIDLFLAGS) /h $@ $<

%_i.c: %.odl
	$(MIDL) $(MIDLFLAGS) /iid $@ $<

%.obj: %.cpp
	$(CC) $(CXXFLAGS) /c /Fo:$@ $<

%.res: %.rc
	$(RC) $(RFLAGS) $<

%.obj: %.cc
	$(CC) $(CXXFLAGS) /c /Fo:$@ $<

%.obj: %.cxx
	$(CC) $(CXXFLAGS) /c /Fo:$@ $<

%.obj: %.c
	$(CC) $(CFLAGS) /c /Fo:$@ $<

%.exe: %.obj
	$(CC) $(CFLAGS) $(LDFLAGS) /Fe:$@ $^ /link $(LINKFLAGS) $(LDLIBS:=.lib)

%.dll: %.obj
	$(CC) $(CFLAGS) $(LDFLAGS) /LD /Fe:$@ $^ /link $(LINKFLAGS)

$(Sources:.cpp=.obj): | NEWS_i.h NEWS_i.c

mpgravity.exe: $(Sources:.cpp=.obj) $(HelperClasses:.cpp=.obj)                 \
        $(ArticleView:.cpp=.obj) $(ThreadView:.cpp=.obj)                       \
        $(GroupView:.cpp=.obj) $(ComposeDlg:.cpp=.obj) $(MainWindow:.cpp=.obj) \
        $(OptionsSetup:.cpp=.obj) $(Rules:.cpp=.obj) $(Binaries:.cpp=.obj)     \
        $(Hunspell:.cxx=.obj) $(Png:.c=.obj) $(Pcre:.c=.obj)                   \
        $(Winface:.c=.obj) $(Zlib:.c=.obj) $(Compface:.c=.obj)                 \
        NEWS.res | NEWS_i.h NEWS_i.c
	$(CC) $(CFLAGS) $(LDFLAGS) /Fe:$@ $^ /link $(LINKFLAGS) $(LDLIBS:=.lib)


clean:
	-cmd.exe /c del /q /f *.exp *.exe *.obj *.pdb *.ilk *.xml *.res *.ipdb *.iobj *.dll *.tmp
	-cmd.exe /c del /q /f zlib\\code\\*.obj
	-cmd.exe /c del /q /f png\\code\\*.obj
	-cmd.exe /c del /q /f pcre\\*.obj
	-cmd.exe /c del /q /f libwinface\\*.obj
	-cmd.exe /c del /q /f hunspell-1.2.8\\src\\hunspell\\*.obj
	-cmd.exe /c del /q /f compface\\*.obj

