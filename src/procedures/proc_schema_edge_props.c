/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_schema_edge_props.h"
#include "proc_schema_props.h"

#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../schema/schema.h"
#include "../graph/graphcontext.h"

// CALL db.schema.relTypeProperties()

ProcedureResult Proc_Schema_EdgeTypePropertiesInvoke(ProcedureCtx *ctx, const SIValue *args) {
	if(array_len((SIValue *)args) != 0) return PROCEDURE_ERR;

	GraphContext *gc = QueryCtx_GetGraphCtx();
	SchemaPropCtx *pdata = rm_malloc(sizeof(SchemaPropCtx));

	pdata->attr_idx = 0;
	pdata->gc = gc;
	pdata->schema = GraphContext_GetSchemaByID(gc, 0, SCHEMA_EDGE);
	pdata->output = array_new(SIValue, 2);
	pdata->output = array_append(pdata->output, SI_ConstStringVal("relType"));
	pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.
	pdata->output = array_append(pdata->output, SI_ConstStringVal("propertyName"));
	pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.

	ctx->privateData = pdata;
	return PROCEDURE_OK;
}

SIValue *Proc_Schema_EdgeTypePropertiesStep(ProcedureCtx *ctx) {
	return Proc_Schema_PropertiesStep(ctx, SCHEMA_EDGE);
}

ProcedureResult Proc_Schema_EdgeTypePropertiesFree(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		SchemaPropCtx *pdata = (SchemaPropCtx *)ctx->privateData;
		array_free(pdata->output);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_Schema_EdgeTypePropertiesCtx() {
	ProcedureOutput **outputs = array_new(ProcedureOutput *, 2);
	ProcedureOutput *output = rm_malloc(sizeof(ProcedureOutput));
	output->name = "relType";
	output->type = T_STRING;
	outputs = array_append(outputs, output);

	output = rm_malloc(sizeof(ProcedureOutput));
	output->name = "propertyName";
	output->type = T_STRING;
	outputs = array_append(outputs, output);

	ProcedureCtx *ctx = ProcCtxNew("db.schema.relTypeProperties",
								   0,
								   outputs,
								   Proc_Schema_EdgeTypePropertiesStep,
								   Proc_Schema_EdgeTypePropertiesInvoke,
								   Proc_Schema_EdgeTypePropertiesFree,
								   NULL,
								   true);
	return ctx;
}