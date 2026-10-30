#!/usr/bin/env python3
"""Checks the menus' translations (frontend/i18n.h).

    python3 tools/i18n_check.py

lang/en.json lists every text of the menus with an id; lang/xx.json has the same ids in another
language. This reports, and exits with 1 when there is anything to report:

  - ids of en.json that a language lacks (shown in English until translated),
  - ids that a language has and en.json does not (left over),
  - texts of en.json that are no longer in the code as they are written there (the code was
    changed, and the translation no longer applies until en.json says the same).

To add a text: write it in the code in English, add it to en.json with a new id (section.words)
and to each language. To change a text: change it in the code and in en.json, and look at the
translations of its id.
"""
import glob
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BS = chr(92)
LITERAL = re.compile('"((?:[^"' + BS + BS + '\\n]|' + BS + BS + '.)*)"')
ADJACENT = re.compile('"(?:[^"' + BS + BS + '\\n]|' + BS + BS + '.)*"(?:\\s*(?://[^\\n]*\\n)?\\s*"(?:[^"' + BS + BS + '\\n]|' + BS + BS + '.)*")*')


def unescape(s):
    return s.replace(BS + '"', '"').replace(BS + BS, BS)


def code_texts():
    """Every string literal of the frontend, and each run of adjacent literals joined."""
    found = set()
    for path in glob.glob(os.path.join(ROOT, 'frontend', '*.[hc]*')):
        text = open(path, encoding='utf-8', errors='replace').read()
        for m in ADJACENT.finditer(text):
            parts = [unescape(p) for p in LITERAL.findall(m.group(0))]
            found.add(''.join(parts))
            found.update(parts)
    return found


def main():
    lang_dir = os.path.join(ROOT, 'lang')
    en = json.load(open(os.path.join(lang_dir, 'en.json'), encoding='utf-8'))
    problems = 0
    for path in sorted(glob.glob(os.path.join(lang_dir, '*.json'))):
        code = os.path.basename(path)[:-5]
        if code == 'en':
            continue
        tr = json.load(open(path, encoding='utf-8'))
        for k in en:
            if not tr.get(k):
                print('%s: missing %s' % (code, k))
                problems += 1
        for k in tr:
            if k not in en:
                print('%s: left over %s' % (code, k))
                problems += 1
    texts = code_texts()
    for k, v in en.items():
        if v not in texts:
            print('en: not in the code as written: %s = %s' % (k, v[:70]))
            problems += 1
    print('%d texts, %d problems' % (len(en), problems))
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
