#ifndef __BASCMD
#define __BASCMD

#include <default.h>

/* Begin Auto-Generated Part */
#ifndef __HAVE_ESQ
#define __HAVE_ESQ
typedef unsigned char ESQ;
#endif
#ifndef __HAVE_CESI
#define __HAVE_CESI
typedef const ESQ*CESI;
#endif
#ifndef __HAVE_ESI
#define __HAVE_ESI
typedef ESQ*ESI;
#endif
#define EStackIndex ESI
#define Quantum ESQ
#ifndef __HAVE_SYM_STR
#define __HAVE_SYM_STR
typedef CESI SYM_STR;
#endif
#ifndef __HAVE_tag_info
#define __HAVE_tag_info
typedef struct{unsigned char tag_order;unsigned char tag_version;unsigned char min_max_args;unsigned char proc_args;void(*CALLBACK tag_proc)();unsigned long tag_str;}tag_info;
#endif
#define primary_tag_list ((const tag_info*const)(_rom_call_addr_hack(467,((*(unsigned long*)((unsigned char*)_rom_call_addr(10A)+16))),200)))
#if MIN_AMS>=101
#define cmd_andpic ({__need_in_use_bit;_rom_call(void,(SYM_STR,CESI,CESI),32A);})
#define cmd_blddata ({__need_in_use_bit;_rom_call(void,(ESI),32B);})
#define cmd_circle ({__need_in_use_bit;_rom_call(void,(CESI,CESI,CESI,CESI),32C);})
#define cmd_clrdraw _rom_call(void,(void),32D)
#define cmd_clrerr ({__need_in_use_bit;_rom_call(void,(void),32E);})
#define cmd_clrgraph ({__need_in_use_bit;_rom_call(void,(void),32F);})
#define cmd_clrhome _rom_call(void,(void),330)
#define cmd_clrio _rom_call(void,(void),331)
#define cmd_clrtable ({__need_in_use_bit;_rom_call(void,(void),332);})
#define cmd_copyvar ({__need_in_use_bit;_rom_call(void,(SYM_STR,SYM_STR),333);})
#define cmd_cubicreg ({__need_in_use_bit;_rom_call(void,(ESI),334);})
#define cmd_custmoff _rom_call(void,(void),335)
#define cmd_custmon _rom_call(void,(void),336)
#define cmd_custom ({__need_in_use_bit;_rom_call(void,(void),337);})
#define cmd_cycle _rom_call(void,(void),338)
#define cmd_cyclepic ({__need_in_use_bit;_rom_call(void,(CESI,CESI,CESI,CESI,CESI),339);})
#define cmd_delfold ({__need_in_use_bit;_rom_call(void,(CESI),33A);})
#define cmd_delvar ({__need_in_use_bit;_rom_call(void,(CESI),33B);})
#define cmd_dialog ({__need_in_use_bit;_rom_call(void,(void),33C);})
#define cmd_disp ({__need_in_use_bit;_rom_call(void,(CESI),33D);})
#define cmd_dispg _rom_call(void,(void),33E)
#define cmd_disphome ({__need_in_use_bit;_rom_call(void,(void),33F);})
#define cmd_disptbl ({__need_in_use_bit;_rom_call(void,(void),340);})
#define cmd_drawfunc ({__need_in_use_bit;_rom_call(void,(CESI),341);})
#define cmd_drawinv ({__need_in_use_bit;_rom_call(void,(CESI),342);})
#define cmd_drawparm ({__need_in_use_bit;_rom_call(void,(CESI),343);})
#define cmd_drawpol ({__need_in_use_bit;_rom_call(void,(CESI),344);})
#define cmd_else _rom_call(void,(void),345)
#define cmd_endfor ({__need_in_use_bit;_rom_call(void,(void),346);})
#define cmd_endloop _rom_call(void,(void),347)
#define cmd_endtry ({__need_in_use_bit;_rom_call(void,(void),348);})
#define cmd_endwhile ({__need_in_use_bit;_rom_call(void,(void),349);})
#define cmd_exit _rom_call(void,(void),34A)
#define cmd_expreg ({__need_in_use_bit;_rom_call(void,(ESI),34B);})
#define cmd_fill ({__need_in_use_bit;_rom_call(void,(CESI,SYM_STR),34C);})
#define cmd_fnoff ({__need_in_use_bit;_rom_call(void,(CESI),34D);})
#define cmd_fnon ({__need_in_use_bit;_rom_call(void,(CESI),34E);})
#define cmd_for ({__need_in_use_bit;_rom_call(void,(SYM_STR,CESI,CESI,CESI),34F);})
#define cmd_get ({__need_in_use_bit;_rom_call(void,(SYM_STR),350);})
#define cmd_getcalc ({__need_in_use_bit;_rom_call(void,(SYM_STR),351);})
#define cmd_goto _rom_call(void,(SYM_STR),352)
#define cmd_graph ({__need_in_use_bit;_rom_call(void,(CESI),353);})
#define cmd_if ({__need_in_use_bit;_rom_call(void,(CESI),354);})
#define cmd_ifthen ({__need_in_use_bit;_rom_call(void,(CESI),355);})
#define cmd_input ({__need_in_use_bit;_rom_call(void,(CESI),356);})
#define cmd_inputstr ({__need_in_use_bit;_rom_call(void,(ESI),357);})
#define cmd_line ({__need_in_use_bit;_rom_call(void,(CESI,CESI,CESI,CESI,CESI),358);})
#define cmd_linehorz ({__need_in_use_bit;_rom_call(void,(CESI,CESI),359);})
#define cmd_linetan ({__need_in_use_bit;_rom_call(void,(CESI,CESI),35A);})
#define cmd_linevert ({__need_in_use_bit;_rom_call(void,(CESI,CESI),35B);})
#define cmd_linreg ({__need_in_use_bit;_rom_call(void,(ESI),35C);})
#define cmd_lnreg ({__need_in_use_bit;_rom_call(void,(ESI),35D);})
#define cmd_local _rom_call(void,(CESI),35E)
#define cmd_lock ({__need_in_use_bit;_rom_call(void,(CESI),35F);})
#define cmd_logistic ({__need_in_use_bit;_rom_call(void,(ESI),360);})
#define cmd_medmed ({__need_in_use_bit;_rom_call(void,(ESI),361);})
#define cmd_movevar ({__need_in_use_bit;_rom_call(void,(SYM_STR,SYM_STR,SYM_STR),362);})
#define cmd_newdata ({__need_in_use_bit;_rom_call(void,(CESI),363);})
#define cmd_newfold ({__need_in_use_bit;_rom_call(void,(SYM_STR),364);})
#define cmd_newpic ({__need_in_use_bit;_rom_call(void,(ESI,SYM_STR,ESI,ESI),365);})
#define cmd_newplot _rom_call(void,(ESI),366)
#define cmd_newprob ({__need_in_use_bit;_rom_call(void,(void),367);})
#define cmd_onevar ({__need_in_use_bit;_rom_call(void,(ESI),368);})
#define cmd_output ({__need_in_use_bit;_rom_call(void,(CESI,CESI,CESI),369);})
#define cmd_passerr ({__need_in_use_bit;_rom_call(void,(void),36A);})
#define cmd_pause ({__need_in_use_bit;_rom_call(void,(CESI),36B);})
#define cmd_plotsoff _rom_call(void,(CESI),36C)
#define cmd_plotson _rom_call(void,(CESI),36D)
#define cmd_popup ({__need_in_use_bit;_rom_call(void,(ESI,ESI),36E);})
#define cmd_powerreg ({__need_in_use_bit;_rom_call(void,(ESI),36F);})
#define cmd_printobj _rom_call(void,(SYM_STR),370)
#define cmd_prompt ({__need_in_use_bit;_rom_call(void,(CESI),371);})
#define cmd_ptchg ({__need_in_use_bit;_rom_call(void,(CESI,CESI),372);})
#define cmd_ptoff ({__need_in_use_bit;_rom_call(void,(CESI,CESI),373);})
#define cmd_pton ({__need_in_use_bit;_rom_call(void,(CESI,CESI),374);})
#define cmd_pttext ({__need_in_use_bit;_rom_call(void,(CESI,CESI,CESI),375);})
#define cmd_pxlchg ({__need_in_use_bit;_rom_call(void,(CESI,CESI),376);})
#define cmd_pxlcircle _rom_call(void,(CESI,CESI,CESI,CESI),377)
#define cmd_pxlhorz _rom_call(void,(CESI,CESI),378)
#define cmd_pxlline _rom_call(void,(CESI,CESI,CESI,CESI,CESI),379)
#define cmd_pxloff ({__need_in_use_bit;_rom_call(void,(CESI,CESI),37A);})
#define cmd_pxlon ({__need_in_use_bit;_rom_call(void,(CESI,CESI),37B);})
#define cmd_pxltext ({__need_in_use_bit;_rom_call(void,(CESI,CESI,CESI),37C);})
#define cmd_pxlvert _rom_call(void,(CESI,CESI),37D)
#define cmd_quadreg ({__need_in_use_bit;_rom_call(void,(ESI),37E);})
#define cmd_quartreg ({__need_in_use_bit;_rom_call(void,(ESI),37F);})
#define cmd_randseed ({__need_in_use_bit;_rom_call(void,(CESI),380);})
#define cmd_rclgdb ({__need_in_use_bit;_rom_call(void,(SYM_STR),381);})
#define cmd_rclpic ({__need_in_use_bit;_rom_call(void,(SYM_STR,CESI,CESI),382);})
#define cmd_rename ({__need_in_use_bit;_rom_call(void,(SYM_STR,SYM_STR),383);})
#define cmd_request ({__need_in_use_bit;_rom_call(void,(CESI,SYM_STR),384);})
#define cmd_return ({__need_in_use_bit;_rom_call(void,(CESI),385);})
#define cmd_rplcpic ({__need_in_use_bit;_rom_call(void,(SYM_STR,CESI,CESI),386);})
#define cmd_send _rom_call(void,(CESI),387)
#define cmd_sendcalc ({__need_in_use_bit;_rom_call(void,(SYM_STR),388);})
#define cmd_sendchat ({__need_in_use_bit;_rom_call(void,(SYM_STR),389);})
#define cmd_shade ({__need_in_use_bit;_rom_call(void,(ESI),38A);})
#define cmd_showstat ({__need_in_use_bit;_rom_call(void,(void),38B);})
#define cmd_sinreg ({__need_in_use_bit;_rom_call(void,(ESI),38C);})
#define cmd_slpline ({__need_in_use_bit;_rom_call(void,(CESI,CESI,CESI),38D);})
#define cmd_sorta ({__need_in_use_bit;_rom_call(void,(ESI),38E);})
#define cmd_sortd ({__need_in_use_bit;_rom_call(void,(ESI),38F);})
#define cmd_stogdb ({__need_in_use_bit;_rom_call(void,(SYM_STR),390);})
#define cmd_stopic ({__need_in_use_bit;_rom_call(void,(ESI),391);})
#define cmd_style ({__need_in_use_bit;_rom_call(void,(CESI,CESI),392);})
#define cmd_table ({__need_in_use_bit;_rom_call(void,(ESI),393);})
#define cmd_text ({__need_in_use_bit;_rom_call(void,(CESI),394);})
#define cmd_toolbar ({__need_in_use_bit;_rom_call(void,(void),395);})
#define cmd_trace _rom_call(void,(void),396)
#define cmd_try ({__need_in_use_bit;_rom_call(void,(void),397);})
#define cmd_twovar ({__need_in_use_bit;_rom_call(void,(ESI),398);})
#define cmd_unlock ({__need_in_use_bit;_rom_call(void,(CESI),399);})
#define cmd_while ({__need_in_use_bit;_rom_call(void,(CESI),39A);})
#define cmd_xorpic ({__need_in_use_bit;_rom_call(void,(SYM_STR,CESI,CESI),39B);})
#define cmd_zoombox _rom_call(void,(void),39C)
#define cmd_zoomdata _rom_call(void,(void),39D)
#define cmd_zoomdec _rom_call(void,(void),39E)
#define cmd_zoomfit _rom_call(void,(void),39F)
#define cmd_zoomin _rom_call(void,(void),3A0)
#define cmd_zoomint _rom_call(void,(void),3A1)
#define cmd_zoomout _rom_call(void,(void),3A2)
#define cmd_zoomprev _rom_call(void,(void),3A3)
#define cmd_zoomrcl _rom_call(void,(void),3A4)
#define cmd_zoomsqr _rom_call(void,(void),3A5)
#define cmd_zoomstd _rom_call(void,(void),3A6)
#define cmd_zoomsto _rom_call(void,(void),3A7)
#define cmd_zoomtrig _rom_call(void,(void),3A8)
#if MIN_AMS>=200
#define cmd_archive ({__need_in_use_bit;_rom_call(void,(CESI),448);})
#define cmd_lu_fact ({__need_in_use_bit;_rom_call(void,(ESI,ESI,ESI,ESI,ESI),484);})
#define cmd_qr_fact ({__need_in_use_bit;_rom_call(void,(ESI,ESI,ESI,ESI),485);})
#define cmd_unarchiv ({__need_in_use_bit;_rom_call(void,(CESI),449);})
#endif
#endif
/* End Auto-Generated Part */

#endif
