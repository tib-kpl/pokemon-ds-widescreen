"""Address-only build fixtures; no ROM or commercial game instructions."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('layout_tables', ROOT / 'tools/soulsilver/layout_tables.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class LayoutTablesTests(unittest.TestCase):
    def test_code_ranges_gaps_function_starts_and_instruction_sets(self):
        fixture = {'sections': [{'name': 'main', 'base': 0x02000000, 'size': 32,
                    'ranges': [[0x02000000, 0x02000008, 1], [0x02000010, 0x02000020, 0]],
                    'named': [0, 0x02000004, 0x0200000c, 0x02000018]}], 'apps': [], 'focus': []}
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            layout = tmp / 'layout.json'
            layout.write_text(json.dumps(fixture))
            module.generate(layout, tmp / 'tables')
            text = (tmp / 'tables/rc_tables.cpp').read_text()
            self.assertIn('{0x02000000u, 0x02000004u, 1, 0, nullptr}', text)
            self.assertIn('{0x02000018u, 0x02000020u, 0, 0, nullptr}', text)
            self.assertNotIn('0x0200000Cu', text)
            self.assertIn('rc_num_chunks = 4', text)
            self.assertNotIn('rc_image_', text)
            self.assertTrue((tmp / 'tables/apps_table.cpp').exists())

    def test_checked_in_layout_covers_every_application_and_focus_probe(self):
        layout = json.loads((ROOT / 'layouts/soulsilver-us.json').read_text())
        sections = {s['name']: s for s in layout['sections']}
        probes = [(r[1], r[2]) for r in layout['apps']] + [(r[1], r[2]) for r in layout['focus']]
        for section, address in probes:
            with self.subTest(section=section, address=hex(address)):
                self.assertTrue(any(a <= address < b for a, b, _ in sections[section]['ranges']))

    def test_portable_tables_keep_bios_sections(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            src = tmp/'source'
            src.mkdir()
            (src/'apps_table.cpp').write_text('// Synthetic application metadata\n')
            (src/'rc_tables.cpp').write_text('const unsigned rc_num_chunks = 0;\nRcSection rc_sections[] = {\n{"main", 0x02000000u, 0x20u, nullptr, nullptr, 0, 0, 0},\n{"bios9", 0xFFFF0000u, 0x20u, nullptr, nullptr, 0, 0, 0},\n};\n')
            subprocess.run([sys.executable, str(ROOT/'tools/soulsilver/tables_only.py'), str(src), str(tmp/'out')], check=True, capture_output=True, text=True)
            self.assertIn('{"bios9", 0xFFFF0000u, 0x20u, nullptr,', (tmp/'out/rc_tables.cpp').read_text())
