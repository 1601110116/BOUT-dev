from __future__ import print_function
from common import *
import sys
from collections import OrderedDict

# read tables
func_tables=OrderedDict()
first_entry=OrderedDict()

with open("tables_cleaned.cxx","r") as f:
    inFunc=0;
    current_table=""
    for line in f:
        inFunc+=line.count('{')
        if inFunc:
            current_table+=line;
        inFunc-=line.count('}')
        if inFunc == 0:
            if not current_table == "":
                #debug(current_table)
                name=current_table.split(" ")[2].split("[")[0]
                #debug("a,",current_table,name,"b")
                if len(name) > 2:
                    #print name
                    func_tables[name]=OrderedDict()
                    for cchar in current_table:
                        debug(cchar,inFunc)
                        if cchar == '}':
                            inFunc-=1
                        if inFunc == 2:
                            current_entry+=cchar
                        if cchar == '{':
                            inFunc+=1
                            current_entry=""
                        if cchar =='}' and inFunc == 1:
                            current_entry=current_entry.split(',')
                            current_entry_cleaned=[]
                            for diff in current_entry:
                                current_entry_cleaned.append(diff.strip())
                            debug(current_entry_cleaned)
                            current_entry_name=current_entry_cleaned[0]
                            debug(name,current_entry_name)
                            #NI not implemented :
                            if current_entry_name=='DIFF_W3':
                                continue
                            if current_entry_name=='DIFF_SPLIT':
                                continue
                            if current_entry_name=='DIFF_DEFAULT':
                                continue
                            #print cn
                            if name not in first_entry:
                                first_entry[name]=current_entry_name
                            if current_entry_cleaned[1:4] == ['NULL']*3:
                                continue
                            debug(name,current_entry_name)
                            func_tables[name][current_entry_name]=current_entry_cleaned[1:4]
                current_table=""
                inFunc=0
descriptions=func_tables.pop("DiffNameTable")
funcname=OrderedDict()
funcname['FirstDerivTable']='indexDD%s'
funcname['FirstStagDerivTable']='indexDD%s'
funcname['SecondDerivTable']='indexD2D%s2'
funcname['SecondStagDerivTable']='indexD2D%s2'
funcname['UpwindTable']='indexVDD%s'
funcname['UpwindStagTable']='indexVDD%s'
funcname['FluxTable']='indexFDD%s'
funcname['FluxStagTable']='indexFDD%s'

funcs_to_gen=[]
default_methods=dict()
# Having a duplicate in the list means something is wrong
duplicates(list(func_tables.keys()))

for t in func_tables:
    debug(func_tables[t], func_tables[t].values(),t)
    fu=next(iter(func_tables[t].values()))
    if fu[0] != "NULL": # not a flux/upwind scheeme
        flux=False
        upwind=False
    else:
        if fu[1] != "NULL":
            upwind=True
        else:
            upwind=False
        flux=True
    if upwind:
        pos=1
    elif flux:
        pos=2
    else:
        pos=0
    if fu[pos][-4:]== "stag":
        stag=True
    else:
        stag=False
    if True:
        duplicates(fields)
        for field in fields:
            duplicates(dirs[field])
            for d in dirs[field]:
                warn()
                try:
                    if not stag:
                        myname=funcname[t]%d.upper()+"_non_stag"
                    else:
                        myname=funcname[t]%d.upper()+"_stag"
                except:
                    import sys
                    print(funcname[t], file=sys.stderr)
                    exit(3)
                if flux:
                    inp="(const "+field+" &v, const "+field+" &f, "
                else:
                    inp="(const "+field+" &f, "
                print("const",field, myname , inp,"CELL_LOC outloc, DIFF_METHOD method) {")
                print("  if (method == DIFF_DEFAULT){")
                print("    method = default_%s_%s;"%(d,t[:-5]+("Deriv" if flux else "")))# drop 'Table'
                print("  }")
                print("  if (outloc == CELL_DEFAULT){")
                print("    outloc = f.getLocation();")
                print("  }")
                print("  switch (method){")
                default_methods["default_%s_%s"%(d,t[:-5])]=func_tables[t]
                duplicates(list(func_tables[t].keys()))
                for method in func_tables[t]:
                    debug(method)
                    print("  case",method,":")
                    if flux:
                        f="v,f"
                    else:
                        f="f"
                    if flux:
                        # f.getLocation() == outloc guaranteed
                        if stag:
                            print("    if (outloc == CELL_%sLOW){"%d.upper())
                            print("      return %s_on_%s(interp_to(v,CELL_CENTRE),f);"% \
                                (funcname[t]%d.upper(),method))
                            print("    } else {") # inloc must be CELL_%sLOW
                            print("      return interp_to(%s_off_%s(v,interp_to(f,CELL_CENTRE)),outloc);"% \
                                (funcname[t]%d.upper(),method))
                            print("    }")
                            stags=['on','off']
                        else:
                            print("    if (v.getLocation() == f.getLocation()){")
                            print("      return interp_to(%s_norm_%s(v,f),outloc);"%(funcname[t]%d.upper(),method))
                            print("    } else {")
                            print("      return interp_to(%s_norm_%s(interp_to(v,CELL_CENTRE),interp_to(f,CELL_CENTRE)),outloc);"%(funcname[t]%d.upper(),method))
                            print("    }")
                            stags=['norm']
                    else: # not flux
                        if stag:
                            print("    if (outloc == CELL_%sLOW){"%d.upper())
                            print("      return %s_on_%s(interp_to(%s,CELL_CENTRE));"%(funcname[t]%d.upper(),method,f))
                            print("    } else {") # inloc must be CELL_%sLOW
                            print("      return interp_to(%s_off_%s(%s),outloc);"% \
                                (funcname[t]%d.upper(),method,f))
                            print("    }")
                            stags=['on','off']
                        else:
                            print("    return interp_to(%s_norm_%s(%s),outloc);"%(funcname[t]%d.upper(),method,f))
                            stags=['norm']
                    for mstag in stags:
                        funcs=func_tables[t][method]
                        if funcs[0]=='NULL':
                            funcs[0]=funcs[1]
                            if funcs[0]=='NULL':
                                funcs[0]=funcs[2]
                        funcs_to_gen.append(["%s_%s_%s"%(funcname[t]%d.upper(),mstag,method),field,d,mstag,funcs[0],flux])
                    print("    break;")
                    #print "    }"
                print("  default:")
                print("    throw BoutException(\"%s AiolosMesh::"%field +myname,'unknown method %d.\\nNote FFTs are not (yet) supported.",method);')
                print("  }; // end switch")
                print("}")
                print()

    #else:
    #    print fu
    #for meth in func_tables[t]:
    #    print meth
    #print func_tables[t]
#print first_entry

headers=""
for func in ["indexDD%s", "indexD2D%s2","indexVDD%s","indexFDD%s"]:
    flux=True
    if func.find("indexD") > -1:
        flux=False
    for field in fields:
        for d in dirs[field]:
            warn()
            sig="("
            if flux:
                sig+="const "+field+" &v,"
            sig += "const "+field+" &f"
            if field=="Field3D" or flux:
                sig+=", CELL_LOC outloc, DIFF_METHOD method";
            if func%d.upper() in ["indexDDZ", "indexD2DZ2"]:
                sig+=",bool ignored";
            elif flux and d in "xy":
                sig+=",REGION ignored";
            #else:
            #    sig+=",REGION region"
            sig+=")"
            function_header="  virtual const "+field+" "+func%d.upper()
            function_header+=sig
            if  not (field == "Field3D" and func[5]=='V' and d == 'z'):
                function_header+=" override;\n"
            else:
                function_header+=""";
virtual const Field3D indexVDD%s(const Field &v,const Field &f, CELL_LOC outloc, DIFF_METHOD method) override{
  return indexVDD%s(dynamic_cast<const Field3D &>(v),dynamic_cast<const Field3D &>(f),outloc,method);
}"""%(d.upper(),d.upper())
            headers+=function_header
            function_header="const "+field+" AiolosMesh::"+func%d.upper()
            function_header+=sig
            if flux:
                f="v, f"
            else:
                f="f"
            print(function_header," {")
            if field != "Field3D" and not flux:
                print("  CELL_LOC outloc=CELL_DEFAULT;")
                print("  DIFF_METHOD method=DIFF_DEFAULT;")
            print("  if (outloc == CELL_DEFAULT) {")
            print("    outloc=f.getLocation();")
            print("  }")
            if flux:
                print("  if (outloc != f.getLocation()) {")
                print('    throw BoutException("AiolosMesh::index?DDX: Unhandled case for shifting.\\n\
f.getLocation()==outloc is required!");')
                print("  }")
            #print '  output.write("Using aiolos mesh for %s\\n");'%(func%d.upper())
            print("  if ((outloc == CELL_%sLOW) != (f.getLocation() == CELL_%sLOW)){"% \
                (d.upper(),d.upper()))
            print("    // we are going onto a staggered grid or coming from one")
            print("    return",func%d.upper()+"_stag","("+f+",outloc,method);")
            print("  } else {")
            print("    return",func%d.upper()+"_non_stag","("+f+",outloc,method);")
            print("  }")
            print("}")
            print()

with open("generated_header.hxx","w") as f:
    f.write(headers)


import sys
#funcs_to_gen=funcs_to_gen[0:2]
tmp=[]
for fu in funcs_to_gen:
    tmp.append(fu[0]+fu[1])
duplicates(tmp)
#seen = set()
#uniq = []
# for x in funcs_to_gen:
#     xs=x[0]+x[1]
#     if xs not in seen:
#         uniq.append(x)
#         seen.add(xs)
# funcs_to_gen=uniq
guards_=[]
sys.stdout=open("generated_stencils.cxx","w")
from gen_stencils import gen_functions_normal
gen_functions_normal(funcs_to_gen)
sys.stdout.flush()
sys.stdout=open("generated_init.cxx","w")

descriptions_cleaned=dict()
debug(descriptions)
for d in descriptions:
    descriptions_cleaned[d]=descriptions[d][1].strip('"');
for d in dirs['Field3D']:
    warn()
    for i in ['First','Second','Upwind','Flux']:
        stags=['','Stag']
        if i in ['First','Second']:
            table="DerivTable"
        else:
            table="Table"
            #stags=[""]
        for stag in stags:
            print('DIFF_METHOD default_%s_%s%sDeriv;'%(d,i,stag))

warn()
print("void AiolosMesh::derivs_init(Options * option) {")
print("  std::string name;")
print("  Options * dirOption;")
for d in dirs['Field3D']:
    print("  output.write(\"\\tSetting derivatives for direction %s:\\n\");"%d)
    print('  dirOption = option->getSection("dd%s");'%d)
    print()
    for i in ['First','Second','Upwind','Flux']:
        stags=['','Stag']
        if i in ['First','Second']:
            table="DerivTable"
        else:
            table="Table"
            #stags=[""]
        for stag in stags:
            warn()
            print('  // Setting derivatives for dd%s and %s'%(d,i+stag))
            print(' ', end=' ')
            if i == 'Second' and stag == "Stag":
                print('    name="C2";')
            if i == 'Flux' or i == 'Upwind':
                default_diff="U1"
            else:
                default_diff="C2"
            for option in ['dirOption']:
                if stag == "Stag":
                    names = [ i+stag,i,"all" ]
                else:
                    names = [ i,"all" ]
                for name in names:
                    print('if (%s->isSet("%s")){'%(option,name))
                    print('    %s->get("%s",name,"%s");'%(option,name,default_diff))
                    print('  } else', end=' ')
            print('  {')
            #elif i == ''
            print('    name="%s";'%default_diff)
            print('  }')
            print(' ', end=' ')
            options=""
            for avail in func_tables[i+stag+table]:
                print('if (strcasecmp(name.c_str(),"%s")==0) {'%avail[5:])
                print('    default_%s_%s%sDeriv = %s;'%(d,i,stag,avail))
                print('    output.write("\t%15s : %s\\n");'%(i+stag,descriptions_cleaned[avail]));
                print('  } else', end=' ')
                options+="\\n * %s"%avail[5:]
            print('{')
            print('    throw BoutException("Dont\'t know what diff method to use for %s (direction %s, tried to use %s)!\\nOptions are:%s",name.c_str());'%(i+stag,d,'%s',options))
            print('  }')
print("}")
#exit(1)
sys.stdout.flush()
