cmake_minimum_required( VERSION 2.6 )

PROJECT(DexScript)

link_directories( )

#ADD_DEFINITIONS()

#ADD_DEFINITIONS(-DDEX_LIBRARY)

set( DexScriptH
    ast/ast.h
    ast/nodes/BaseNode.h
    ast/nodes/ControlNodes.h
    ast/nodes/DataNodes.h
    ast/nodes/LookupNodes.h
    ast/nodes/nodes.h
    ast/nodes/OperNodes.h
    bytecode/bytes.h
    datastruct/dicttest.h
    datastruct/dicttools.h
    datastruct/settools.h
    DexRef.h
    DexScr.h
    interp/commandline.h
    interp/error.h
    interp/global.h
    interp/intern.h
    interp/interp_state.h
    interp/objstack.h
    interp/path.h
    interp/VarContext.h
    libs/draw/dexdraw.h
    libs/draw/deximage.h
    libs/draw/dexkeys.h
    libs/rand/mtwist.h
    mainh.h
    objdef/dex_conv.h
    parsing/keywords.h
    parsing/lex.h
    parsing/nparse.h
    parsing/op.h
    parsing/parse_structs.h
    stdobjs/basic.h
    stdobjs/consts.h
    stdobjs/iters.h
    stdobjs/math.h
    stdobjs/methods.h
    stdobjs/objdefutil.h
    stdobjs/str.h
    stdobjs/struct.h
    stdobjs/types.h
    types/dexbool.h
    types/dexdict.h
    types/dexlist.h
    types/dexmodule.h
    types/dexnum.h
    types/dexnumops.h
    types/dexobj.h
    types/dexset.h
    types/dexstdops.h
    types/dexstr.h
    types/dexstrops.h
    types/dextype.h
    types/dexuobj.h
    types/funcs/dexbound.h
    types/funcs/dexbuiltin.h
    types/funcs/dexfunc.h
    types/funcs/dexgen.h
    types/init.h
    types/io/dexfile.h
    types/typetools.h
    types/util/hashutil.h
    types/util/serialize.h
    types/util/strmake.h
    types/util/VarToVect.h
    types/util/VoidDict.h
	)
	
set( DexScriptCPP	
   ast/ast.cpp
   ast/nodes/BaseNode.cpp
   ast/nodes/ControlNodes.cpp
   ast/nodes/DataNodes.cpp
   ast/nodes/LookupNodes.cpp
   ast/nodes/OperNodes.cpp
   bytecode/bytes.cpp
   DexRef.cpp
   interp/error.cpp
   interp/interp_state.cpp
   interp/path.cpp
   interp/VarContext.cpp
   libs/draw/dexdraw.cpp
   libs/draw/deximage.cpp
   libs/rand/mtwist.cpp
   mainf.cpp
   parsing/keywords.cpp
   parsing/lex.cpp
   parsing/nparse.cpp
   parsing/op.cpp
   parsing/shuntparse.cpp
   stdobjs/basic.cpp
   stdobjs/consts.cpp
   stdobjs/iters.cpp
   stdobjs/math.cpp
   stdobjs/methods.cpp
   stdobjs/str.cpp
   stdobjs/struct.cpp
   stdobjs/types.cpp
   types/collect/dexheap.cpp
   types/dexbool.cpp
   types/dexdict.cpp
   types/dexlist.cpp
   types/dexmodule.cpp
   types/dexnone.cpp
   types/dexnum.cpp
   types/dexobj.cpp
   types/dexset.cpp
   types/dexstdops.cpp
   types/dexstr.cpp
   types/dexstrops.cpp
   types/dextype.cpp
   types/dexuobj.cpp
   types/funcs/dexbound.cpp
   types/funcs/dexbuiltin.cpp
   types/funcs/dexfunc.cpp
   types/funcs/dexgen.cpp
   types/init.cpp
   types/io/dexfile.cpp
   types/util/serialize.cpp
   types/util/VoidDict.cpp
  )
  
add_executable( DexScript
                ${DexScriptH} ${DexScriptCPP} )

#add_library( DexScript
#                ${DexScriptH} ${DexScriptCPP} )
