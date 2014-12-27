/**
 * The contents of this file are only a suggestion.
 * Feel free to change anything.
 **/

#include "matcher.h"
#define PLUS ('+')
#define DOT ('.')
#define BACKSLASH ('\\')
#define QMARK ('?')
#define NEWLINE ('\n')
#define SPACE (' ')

int q_check(char prev, char *pattern, char *p_line);

int plus_trail(char prev, char *pattern, char *p_line);
/**
 * Returns true if partial_line matches pattern, starting from
 * the first char of partial_line.
 **/
int matches_leading(char *partial_line, char *pattern) {
	// You can use this recommended helper function, or not.
	if (!pattern[0]) {
		return 1;
	} else if (pattern[0] == BACKSLASH) {
		if (pattern[2] == QMARK) {
			return q_check(pattern[1], pattern + 3, partial_line);
		} else if (pattern[2] == PLUS) {
			return plus_trail(pattern[1], pattern + 3, partial_line);
		} else if (*partial_line && pattern[1] == *partial_line) {
			return matches_leading(partial_line + 1, pattern + 2);
		} else {
			return 0;
		}
	} else if (pattern[1] == PLUS) {
		return plus_trail(pattern[0], pattern + 2, partial_line);
	} else if (pattern[1] == QMARK) {
		return q_check(pattern[0], pattern + 2, partial_line);
	} else if (*partial_line && (pattern[0] == DOT || pattern[0] == *partial_line)) {
		return matches_leading(partial_line + 1, pattern + 1);
	} else {
		return 0;
	}
}

/**
 * prev: the repeating character
 * pattern: character after '+'
 * p_line: the line to find matches in
 * check for '\++\+': ++, +++, ++++, etc
 **/
int plus_trail(char prev, char *pattern, char *p_line) {
	while (*p_line && (*p_line == prev || prev == DOT)) {
		p_line++;
		if (matches_leading(p_line, pattern)) {
			return 1;
		}
	}
	return 0;
}

/**
 * prev: the optional character
 * pattern: character after '?'
 * p_line: the line to find matches in
 **/
int q_check(char prev, char *pattern, char *p_line) {
	if (*p_line && (*p_line == prev || prev == DOT)) {
		p_line++;
		if (*pattern != BACKSLASH && *pattern == prev) { // Accounts for 'a?a'
			return matches_leading(p_line, pattern + 1);
		} else if (*pattern == BACKSLASH && *(pattern + 1) == prev) { // Accounts for '\\?\+'
			return matches_leading(p_line, pattern + 2);
		} else {
			return matches_leading(p_line, pattern); // Accounts for 'a?b' or '\+?b'
		}
	} else {
		return matches_leading(p_line, pattern);
	}
}

/**
 * Implementation of your matcher function, which
 * will be called by the main program.
 *
 * You may assume that both line and pattern point
 * to reasonably short, null-terminated strings.
 **/
int rgrep_matches(char *line, char *pattern) {
	while (*line) {
		if (matches_leading(line, pattern)) {
			return 1;
		}
		line++;
	}
	return 0;
}
