/************************************************************************
*
* Copyright 2012 Jonatan Liljedahl <lijon@kymatica.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
************************************************************************/

#include "GC.h"
#include "PyrKernel.h"
#include "PyrPrimitive.h"
#include "PyrSymbol.h"
#include "SCBase.h"
#include "SC_DirUtils.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <cerrno>

//extern "C" {
#include "SCDoc.h"
//}

PyrSymbol *s_scdoc_node;

static void _doc_traverse(struct VMGlobals* g, DocNode *n, PyrObject *parent, PyrSlot *slot)
{
    PyrObject *result = instantiateObject( g->gc, s_scdoc_node->u.classobj, 0, false, false );
    result->size = 0;
	SetObject(slot, result);
	if(parent) g->gc->GCWrite(parent, result);

    PyrSymbol *id = getsym(n->id);
    SetSymbol(result->slots+result->size++, id);

    if(n->text) {
        PyrObject *str = (PyrObject*) newPyrString(g->gc, n->text, 0, true);
        SetObject(result->slots+result->size++, str);
        g->gc->GCWrite(result, str);
    } else {
        SetNil(result->slots+result->size++);
    }

    if(n->n_childs) {
        PyrObject *array = newPyrArray(g->gc, n->n_childs, 0, true);
        array->size = 0;
        SetObject(result->slots+result->size++, array);
        g->gc->GCWrite(result, array);
        for(int i=0; i<n->n_childs; i++) {
            array->size++;
            _doc_traverse(g, n->children[i], array, array->slots+i);
        }
    } else {
        SetNil(result->slots+result->size++);
    }
    result->size += 3; // makeDiv, notPrivOnly, sort
}


int prSCDoc_ParseFile(struct VMGlobals* g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
    char filename[PATH_MAX];
    int mode, err;

	a = g->sp - 2;
	b = g->sp - 1;
	c = g->sp;

    err = slotStrVal(b, filename, PATH_MAX);
    if (err) return err;
    
    err = slotIntVal(c, &mode);
    if (err) return err;

    DocNode *n = scdoc_parse_file(filename, mode);
    if(n) {
//        doc_node_dump(n);
        _doc_traverse(g, n, NULL, a);
        doc_node_free_tree(n);
    } else {
        SetNil(a);
    }
    return errNone;
}

void initSCDocPrimitives()
{
	int base, index = 0;
	
	s_scdoc_node = getsym("SCDocNode");

	base = nextPrimitiveIndex();

	definePrimitive(base, index++, "_SCDoc_ParseFile", prSCDoc_ParseFile, 3, 0);

}

