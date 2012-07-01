#! /usr/bin/env python
# -*- coding: utf-8 -*-

"""stats.py
Skript pro zobrazeni statistik zdrojoveho kodu jazyka C++

Autori:

    Cerny Lukas          <xcerny37@stud.fit.vutbr.cz>
    Dvorak Miroslav      <xdvora11@stud.fit.vutbr.cz>
    Mrazek Petr          <xmraze03@stud.fit.vutbr.cz>
    Oujesky Miroslav     <xoujes00@stud.fit.vutbr.cz>

odladeno v interpretru Pythonu verze 2.4.4 a 2.5.2

verze: $Rev: 44 $
"""

from glob import glob
import re
import sys, os

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 

class Parser:
    """Trida zaobalujici prochazeni souboru a vypocet podle daneho rezimu"""
    
    # klicova slova jazyka c++
    keywords = ['asm', 'auto', 'bool', 'break', 'case', 'catch', 'char', 
                'class', 'const', 'const_cast', 'continue', 'default', 
                'delete', 'do', 'double', 'dynamic_cast', 'else', 'enum',
                'explicit', 'export', 'extern', 'false', 'float', 'for', 
                'friend', 'goto', 'if', 'inline', 'int', 'long', 'mutable',
                'namespace', 'new', 'operator', 'private', 'protected', 
                'public', 'register', 'reinterpret_cast', 'return', 'short',
                'signed', 'sizeof', 'static', 'static_cast', 'struct', 'switch',
                'template', 'this', 'throw', 'true', 'try', 'typedef', 'typeid',
                'typename', 'union', 'unsigned', 'using', 'virtual', 'void', 
                'volatile', 'wchar_t', 'while']
    
    operators = ['=', '+', '-', '/', '*', '%', '+=', '-=', '*=', '/=', '%=', 
                 '>>=', '<<=', '&=', '^=', '|=', '++', '--', '==', '!=', '>', 
                 '<', '>=', '<=', '!', '&&', '||',  '&', '|', '^', '~', '<<', 
                 '>>', 'sizeof', '::', '->', '.', '->*', '.*', 'new', 
                 'typeid', 'delete']
                 
    # rezim parseru
    mode = ''
    
    # hledany vzor
    pattern = ''
    
    # soubor
    file = None
    
    # regularni vyraz pro blokovy komentar
    re_comment_b = re.compile(r'(/\*.*?\*/)', re.S)
    
    # regularni vyraz pro radkovy komentar
    re_comment_l = re.compile(r'(//.*)\n')
    
    # regularni vyraz pro retezec
    re_str = re.compile(r'(".*?[^\\"]")', re.S)
    
    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def __init__(self, file_name, mode, pattern):
        """Konstruktor"""        
        
        self.mode = mode
        self.pattern = pattern
        
        try:
            self.file = open(file_name, 'r')
        except IOError:
            msg = 'Chyba: soubor ' + file_name + ' nelze otevrit'
            print >> sys.stderr, msg

    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def __del__(self):
        """Destruktor"""
        if (self.file):
            self.file.close()

    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def count(self):
        """Tato metoda vraci pocet vyskytu podle daneho rezimu"""
        
        if (not self.file):
            return 0        
        elif (self.mode == 'keywords'):
            file_content = self._strip_file()
            return self._count_keywords(file_content)
        elif (self.mode == 'pattern'):
            return self._count_pattern()
        elif (self.mode == 'operators'):
            file_content = self._strip_file()
            return self._count_operators(file_content)
        elif (self.mode == 'identifiers'):
            file_content = self._strip_file()
            return self._count_identifiers(file_content)
        elif (self.mode == 'identifiers_k'):
            file_content = self._strip_file()
            return self._count_identifiers_k(file_content)
        elif (self.mode == 'comments'):
            return self._count_comments()
        else:
            return 0
    
    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def _strip_comments(self, file_content):
        """odstrani komentare z nacteneho souboru"""
        # odstraneni blokovych komentaru
        file_content = self.re_comment_b.sub('/**/', file_content)
        
        # odstraneni radkovych komentaru
        file_content = self.re_comment_l.sub('', file_content)
        
        return file_content
    
    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def _strip_strings(self, file_content):
        """odstrani retezce z nacteneho souboru"""
        file_content = self.re_str.sub('""', file_content)
        
        return file_content
    
    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def _strip_file(self):
        """Odstrani ze souboru komentare a retezce"""
        file_content = self.file.read()
        
        # odstraneni retezcu
        file_content = self._strip_comments(file_content)
        
        file_content = self._strip_strings(file_content)
        
        return file_content

    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def _count_keywords(self, file_content):
        """Spocita klicova slova v souboru"""
        re_keywords = re.compile(r'(^|[-*+=/\\()\[\] \t\n#"\'.,{}|<>%&;:?!^])(' 
                                 + r'|'.join(self.keywords) 
                                 + r')($|[-*+=/\\()\[\] \t\n#"\'.,{}|<>%&;:?!^])', re.M)
        
        return len(re_keywords.findall(file_content))
    
    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def _count_operators(self, file_content):
        """Spocita operatory v souboru"""
        
        re_operators = re.compile(r'(^|[a-zA-Z0-9()\[\]{} \n\t\r])('
                                  + r'|'.join(map(re.escape, self.operators))
                                  + r')($|[a-zA-Z0-9()\[\]{} \n\t\r])', re.M)
        
        return len(re_operators.findall(file_content))
    
    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def _count_identifiers(self, file_content):
        """Spocita identifikatory bez klicovych slov v souboru"""
        
        count_i = self._count_identifiers_k(file_content)
        count_k = self._count_keywords(file_content)
        
        return count_i - count_k
        
        
    
    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def _count_identifiers_k(self, file_content):
        """Spocita identifikatory s klicovymi slovy v souboru"""
        
        re_identifiers = re.compile(r'[a-zA-Z_][a-zA-Z0-9_]*')
        
        return len(re_identifiers.findall(file_content))

    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def _count_comments(self):
        """Spocita pocet bytu komentaru v souboru"""
        file_content = self.file.read()
        
        comments_b = self.re_comment_b.findall(file_content)
        comments_l = self.re_comment_l.findall(file_content)
        
        count = 0
        
        for comment in comments_b:
            count += len(comment)
        
        for comment in comments_l:
            count += len(comment)
        
        return count
    
    #  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
    def _count_pattern(self):
        """Vraci pocet vyskytu retezce pattern v souboru"""
        count = 0
        
        for line in self.file:
            count += line.count(self.pattern)
        
        return count

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
def usage(error = None):
    """Vypise napovedu a pripadne chybovou hlasku"""
    
    usage_msg = u"""Pouziti: stats.py [parametr]

Parametry:
  --help      Vypise tuto napovedu a skonci
  -k          Vypise pocet klicovych slov
  -o          Vypise pocet operatoru
  -ik         Vypise pocet identifikatoru vcetne klicovych slov
  -i          Vypise pocet identifikatoru vyjma klicovych slov
  -w RETEZEC  Vyhleda RETEZEC a vypise pocet vyskytu
  -c          Vypise celkovou delku komentaru v B
  -p          Vypise pouze nazev souboru a ne celou cestu"""

    if (error != None):
        print >> sys.stderr, error 
        print >> sys.stderr, usage_msg
    else:
        print usage_msg
    
    return

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
def main(argv):
    """Hlavni funkce skriptu"""
    
    # dulezite promenne
    mode = ''
    pattern = ''
    no_path = False
    
    # zpracovani parametru vstupu
    if (len(argv) < 2):
        usage('Vyberte jeden z povinnych parametru')
        return 1
    
    if (argv[1] == '--help'):
        usage()
        return 0
    
    if (argv[1] == '-p'):
        argv.pop(1)
        no_path = True
        if (len(argv) < 2):
            usage('Vyberte jeden z povinnych parametru')
            return 1
    
    if (argv[1] == '-k'):
        mode = 'keywords'
    elif (argv[1] == '-o'):
        mode = 'operators'
    elif (argv[1] == '-ik'):
        mode = 'identifiers_k'
    elif (argv[1] == '-i'):
        mode = 'identifiers'
    elif (argv[1] == '-c'):
        mode = 'comments'
    elif (argv[1] == '-w'):
        mode = 'pattern'
        if (len(argv) > 2):
            pattern = argv[2]
            argv.pop(2)
        else:
            usage('Parametr -w vyzaduje zadanou hodnotu RETEZEC')
            return 1
    else:
        usage('Neznamy parametr')
        return 1
    
    if (len(argv) > 2 and argv[2] == '-p' and no_path == False):
        no_path = True
        argv.pop(2)

    if (len(argv) > 2):
        usage('Prilis mnoho parametru')
        return 1
            

    # ziskame seznam souboru se kterymi budeme pracovat    
    files = get_src_files(os.getcwd())

    # projdeme soubory
    result = parse_files(files, no_path, mode, pattern)
    
    # vytiskneme vysledek
    print_result(result)
    
    return 0

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
def get_src_files(path):
    """Vraci pole se jmeny souboru v aktualni slozce a podslozkach, ktere maji
    priponu .cpp nebo .h"""

    # pripony vstupnich souboru, ktere chceme do seznamu
    target_ext = ['.cpp', '.h']    
    
    files = glob(os.path.join(path, '*'))
    result = []
    
    for file_name in files:
        
        if (os.path.isdir(file_name)):
            result += get_src_files(file_name)
        else:
            ext = os.path.splitext(file_name)[1]
            if (ext in target_ext):
                result.append(file_name)
    
    return result

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
def parse_files(files, no_path, mode, pattern):
    """Vraci seznam s vysledky vypoctu podle daneho rezimu"""

    result = []
    
    for file_name in files:
        
        if (no_path):
            file_str = os.path.basename(file_name)
        else:
            file_str = file_name
                
        parser = Parser(file_name, mode, pattern)            
        result.append((file_str, parser.count()))
    
    return result

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
def print_result(results):    
    """Vypise vysledky skriptu na standardni vystup"""
    
    # celkovy soucet hodnot
    total = sum([row[1] for row in results])
    total_len = len(str(total))
    
    # pridame celkovy pocet do pole pro vystup
    results.append(('CELKEM:', total))
    
    # zjistime nejdelsi nazev souboru
    longest_file_len  = 0;
    
    for row in results:
        if (len(row[0]) > longest_file_len):
            longest_file_len = len(row[0])
    
    # vypiseme jednotlive radky
    for record in results:
        print format_record(record[0], record[1], longest_file_len, total_len)

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
def format_record(file_name, count, max_file, max_count):
    """Vraci formatovany radek vystupu s korektnim poctem mezer"""
    spaces = 1 + (max_file - len(file_name)) + (max_count - len(str(count)))    
    return file_name + (' ' * spaces) + str(count)

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
if __name__ == "__main__":    
    sys.exit(main(sys.argv))
    
# eof