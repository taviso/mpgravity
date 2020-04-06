include GNUmakefile.common

CPPFLAGS +=/I include /D MICROPLANET /D MP_PACKBYTE /D _USE_32BIT_TIME_T
CPPFLAGS +=/D HUNSPELL_STATIC /D PCRE_STATIC
LDLIBS   +=comctl32 winmm version winface compface zlib pcre hunspell png
LINKFLAGS+=/LIBPATH:libwinface /LIBPATH:compface /LIBPATH:zlib /LIBPATH:pcre
LINKFLAGS+=/LIBPATH:hunspell /LIBPATH:png

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

$(Sources:.cpp=.obj): | NEWS_i.h NEWS_i.c
$(HelperClasses:.cpp=.obj): | NEWS_i.h NEWS_i.c
$(ArticleView:.cpp=.obj): | NEWS_i.h NEWS_i.c
$(ThreadView:.cpp=.obj): | NEWS_i.h NEWS_i.c
$(GroupView:.cpp=.obj): | NEWS_i.h NEWS_i.c
$(ComposeDlg:.cpp=.obj): | NEWS_i.h NEWS_i.c
$(MainWindow:.cpp=.obj): | NEWS_i.h NEWS_i.c
$(OptionsSetup:.cpp=.obj): | NEWS_i.h NEWS_i.c
$(Rules:.cpp=.obj): | NEWS_i.h NEWS_i.c
$(Binaries:.cpp=.obj): | NEWS_i.h NEWS_i.c

mpgravity.exe: $(Sources:.cpp=.obj) $(HelperClasses:.cpp=.obj)                 \
        $(ArticleView:.cpp=.obj) $(ThreadView:.cpp=.obj)                       \
        $(GroupView:.cpp=.obj) $(ComposeDlg:.cpp=.obj) $(MainWindow:.cpp=.obj) \
        $(OptionsSetup:.cpp=.obj) $(Rules:.cpp=.obj) $(Binaries:.cpp=.obj)     \
        NEWS.res
	make -C libwinface
	make -C compface
	make -C zlib
	make -C pcre
	make -C hunspell
	make -C png
	$(CC) $(CFLAGS) $(LDFLAGS) /Fe:$@ $^ /link $(LINKFLAGS) $(LDLIBS:=.lib)


clean::
	-cmd.exe /c del /q /f NEWS_i.h NEWS_i.c
	make -C libwinface clean
	make -C compface clean
	make -C zlib clean
	make -C pcre clean
	make -C hunspell clean
	make -C png clean
