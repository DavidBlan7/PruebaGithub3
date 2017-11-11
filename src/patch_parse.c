
#include "patch_parse.h"

static int parse_header_start(git_patch_parsed *patch, git_patch_parse_ctx *ctx)
{
	if (parse_header_path(&patch->header_old_path, ctx) < 0)
		return parse_err("corrupt old path in git diff header at line %"PRIuZ,
			ctx->line_num);

	if (parse_advance_ws(ctx) < 0 ||
		parse_header_path(&patch->header_new_path, ctx) < 0)
		return parse_err("corrupt new path in git diff header at line %"PRIuZ,
			ctx->line_num);

	return 0;
}

typedef enum {
	STATE_START,

	STATE_DIFF,
	STATE_FILEMODE,
	STATE_MODE,
	STATE_INDEX,
	STATE_PATH,

	STATE_SIMILARITY,
	STATE_RENAME,
	STATE_COPY,

	STATE_END,
} parse_header_state;

	parse_header_state expected_state;
	parse_header_state next_state;
} parse_header_transition;

static const parse_header_transition transitions[] = {
	/* Start */
	{ "diff --git "         , STATE_START,      STATE_DIFF,       parse_header_start },

	{ "deleted file mode "  , STATE_DIFF,       STATE_FILEMODE,   parse_header_git_deletedfilemode },
	{ "new file mode "      , STATE_DIFF,       STATE_FILEMODE,   parse_header_git_newfilemode },
	{ "old mode "           , STATE_DIFF,       STATE_MODE,       parse_header_git_oldmode },
	{ "new mode "           , STATE_MODE,       STATE_END,        parse_header_git_newmode },

	{ "index "              , STATE_FILEMODE,   STATE_INDEX,      parse_header_git_index },
	{ "index "              , STATE_DIFF,       STATE_INDEX,      parse_header_git_index },
	{ "index "              , STATE_END,        STATE_INDEX,      parse_header_git_index },

	{ "--- "                , STATE_INDEX,      STATE_PATH,       parse_header_git_oldpath },
	{ "+++ "                , STATE_PATH,       STATE_END,        parse_header_git_newpath },
	{ "GIT binary patch"    , STATE_INDEX,      STATE_END,        NULL },
	{ "Binary files "       , STATE_INDEX,      STATE_END,        NULL },

	{ "similarity index "   , STATE_DIFF,       STATE_SIMILARITY, parse_header_similarity },
	{ "dissimilarity index ", STATE_DIFF,       STATE_SIMILARITY, parse_header_dissimilarity },
	{ "rename from "        , STATE_SIMILARITY, STATE_RENAME,     parse_header_renamefrom },
	{ "rename old "         , STATE_SIMILARITY, STATE_RENAME,     parse_header_renamefrom },
	{ "copy from "          , STATE_SIMILARITY, STATE_COPY,       parse_header_copyfrom },
	{ "rename to "          , STATE_RENAME,     STATE_END,        parse_header_renameto },
	{ "rename new "         , STATE_RENAME,     STATE_END,        parse_header_renameto },
	{ "copy to "            , STATE_COPY,       STATE_END,        parse_header_copyto },

	/* Next patch */
	{ "diff --git "         , STATE_END,        0,                NULL },
	{ "@@ -"                , STATE_END,        0,                NULL },
	{ "-- "                 , STATE_END,        0,                NULL },
	parse_header_state state = STATE_START;
	for (; ctx->remain_len > 0; parse_advance_line(ctx)) {
		for (i = 0; i < ARRAY_SIZE(transitions); i++) {
			const parse_header_transition *transition = &transitions[i];
			size_t op_len = strlen(transition->str);
			if (transition->expected_state != state ||
			    memcmp(ctx->line, transition->str, min(op_len, ctx->line_len)) != 0)
			state = transition->next_state;

			if (transition->fn == NULL)
			if ((error = transition->fn(patch, ctx)) < 0)

	if (state != STATE_END) {
		error = parse_err("unexpected header line %"PRIuZ, ctx->line_num);
		goto done;
	}
