#!/usr/bin/env python3

import rdflib
import sys
import string

# incontextsensing

# 140x52 = 7420
# 170x15 = 2720
# 190x11 = 2280
# 194x10 = 2134
# 200x9 = 2000
# 197x9 = 1970
#DICT_SLOTS = 197
#DICT_SLOT_WIDTH = 9

# incontextsensing_short
DICT_SLOTS = 53
DICT_SLOT_WIDTH = 9


dictionary = [] #[{}] * DICT_SLOTS
for i in range(DICT_SLOTS):
	dictionary.append({})

tuples = []

def insert_tuple(s, p, o):
	#print(s, p, o)
	ks = insert_dictionary(s)
	kp = insert_dictionary(p)
	ko = insert_dictionary(o)
	tuples.append((ks, kp, ko))

def hash(s):
	# SDBM
	h = 0
	for x in s:
		h = (x) + (h << 6) + (h << 16) - h;
		h %= 2 ** 32
	return h

def find_dictionary(s, meta):
	#for i, v in enumerate(dictionary):
	i0 = hash(s) % DICT_SLOTS
	#print("// i0({}) = {}".format(s, i0))
	iend = i0 - 1
	if iend < 0: iend = DICT_SLOTS - 1
	i = i0

	while True:
		v = dictionary[i]
		if 'refcount' in v:
			if v['meta'] == meta and v['s'] == s: return i

		i += 1
		i %= DICT_SLOTS
		if i == iend: break;
	return -1

def find_free(s):
	i0 = hash(s) % DICT_SLOTS
	iend = i0 - 1
	if iend < 0: iend = DICT_SLOTS - 1
	i = i0
	while True:
		v = dictionary[i]
		if not v: return i
		i += 1
		i %= DICT_SLOTS
		if i == iend: break;
	#for i, v in enumerate(dictionary):
		#if not v: return i
	return -1


def insert_dictionary(s):
	new_meta_entry = { 's': bytes(), 'refcount': 1, 'meta': True }
	i = 0
	j = 0
	while i < len(s):
		part = bytes(s[i:i + DICT_SLOT_WIDTH], 'latin1')
		part += (b'\x00' * (DICT_SLOT_WIDTH - len(part)))
		assert len(part) == DICT_SLOT_WIDTH
		k = find_dictionary(part, meta = False)
		if k == -1:
			# not found
			k = find_free(part)
			#print_dictionary()
			assert k != -1
			dictionary[k]['refcount'] = 1
		else:
			dictionary[k]['refcount'] += 1

		dictionary[k]['s'] = part #, 'latin1')
		dictionary[k]['meta'] = False

		#new_meta_entry['s'][j] = bytes(chr(k), 'latin1')
		new_meta_entry['s'] += bytes(chr(k), 'latin1') # + new_meta_entry['s'][j+1:]
		#print(len(new_meta_entry['s']))
		assert len(bytes(chr(k), 'latin1')) == 1

		#print( len(new_meta_entry['s']) )
		#print(s, len(s))

		i += DICT_SLOT_WIDTH
		j += 1

	new_meta_entry['s'] += (b'\xff' * (DICT_SLOT_WIDTH - len(new_meta_entry['s'])))
	assert len(new_meta_entry['s']) == DICT_SLOT_WIDTH

	km = find_dictionary(new_meta_entry['s'], True)
	if km == -1:
		km = find_free(new_meta_entry['s'])
		#print_dictionary()
		assert km != -1
		dictionary[km] = new_meta_entry
	else:
		dictionary[km]['refcount'] += 1

	return km

def print_dictionary():
	for i, v in enumerate(dictionary):
		if not v: continue
		print("[{}]: {} x {} meta={}".format(i, v['refcount'], v['s'], v['meta']))


bnode_map = {}
bnode_i = 0

def to_string(x):
	global bnode_i
	if isinstance(x, rdflib.term.BNode):
		s = str(x)
		if s not in bnode_map:
			bnode_map[s] = "_:" + "{:x}".format(bnode_i)
			bnode_i += 1
		return bnode_map[s]

	elif isinstance(x, rdflib.term.URIRef):
		return "<" + str(x) + ">"

	else:
		return '"' + str(x) + '"'

def to_quoted_cpp_string(s):
	r = ''
	for x in s:
		if chr(x) in string.printable and chr(x) not in '"\t\n\r\x0b\x0c':
			r += chr(x)
		elif x == '"':
			r += '\\"'
		else:
			r += '\\x{:02x}'.format(x)
	return r

def dict_to_cpp_string():
	ss = ''

	for i in range(DICT_SLOTS):
		if not dictionary[i]:
			ss += '             "' + r'\x00' * (DICT_SLOT_WIDTH + 1) + '"\n'
		else:
			v = dictionary[i]
			assert v['refcount'] <= 127
			ss += '/* {:02x} */ "\\x{:02x}{}"\n'.format(
					i,
					v['refcount'] + (v['meta'] * 128),
					to_quoted_cpp_string(v['s'])
			)

	return '''
#define STATIC_DICTIONARY_OUTSOURCE 1
#define STATIC_DICTIONARY_SLOTS {}
#define STATIC_DICTIONARY_SLOT_WIDTH {}
#include <util/tuple_store/static_dictionary.h>

typedef wiselib::StaticDictionary<Os, STATIC_DICTIONARY_SLOTS, STATIC_DICTIONARY_SLOT_WIDTH> PrecompiledDictionary;

/*
        #if STATIC_DICTIONARY_OUTSOURCE
                dictionary_->set_data(dict_data_);
        #endif
*/

const char dict_data_[{} + 1 /* for 0-byte at end of string */] =
{};
    '''.format(DICT_SLOTS, DICT_SLOT_WIDTH, DICT_SLOTS * (DICT_SLOT_WIDTH + 1), ss)


g = rdflib.Graph()
#g.parse('test.rdf', format='n3')
g.parse('incontextsensing_short.rdf', format='n3')
for s, p, o in g:
	insert_tuple(to_string(s), to_string(p), to_string(o))

#print_dictionary()

# Generate dict string
b = bytes()
for i in range(DICT_SLOTS):
	if not dictionary[i]:
		b += (b'\x00' * (DICT_SLOT_WIDTH + 1))
	else:
		v = dictionary[i]
		assert v['refcount'] <= 127
		b += bytes(chr(v['refcount'] + (v['meta'] * 128)), 'latin1')
		#print("{} l={}".format(v['s'], len(v['s'])))
		assert len(v['s']) == DICT_SLOT_WIDTH
		b += v['s']
	#sys.stderr.write(str(len(b)) + '\n')

#print(b)

W = (DICT_SLOT_WIDTH + 1)

ss = '"';
i = 0
for bt in b:
	ss += '\\x{:02x}'.format(int(bt))
	i += 1
	if i >= W:
		ss += '"\n"'
		i = 0
ss += '"'

print(dict_to_cpp_string())

#print(s_dict)
#print('''
##define STATIC_DICTIONARY_OUTSOURCE 1
##define STATIC_DICTIONARY_SLOTS {}
##define STATIC_DICTIONARY_SLOT_WIDTH {}
##include <util/tuple_store/static_dictionary.h>

#typedef wiselib::StaticDictionary<Os, STATIC_DICTIONARY_SLOTS, STATIC_DICTIONARY_SLOT_WIDTH> PrecompiledDictionary;

#/*
	##if STATIC_DICTIONARY_OUTSOURCE
		#dictionary_->set_data(dict_data_);
	##endif
#*/

#const char dict_data_[{} + 1 /* for 0-byte at end of string */] =
#{};
#'''.format(DICT_SLOTS, DICT_SLOT_WIDTH, len(b), ss))

ss = '{ '
ks, kp, ko = tuples[0]
ss += '{}, {}, {}'.format(ks, kp, ko)
for ks, kp, ko in tuples[1:]:
	ss += ', {}, {}, {} // (%s %s %s)\n'.format(ks, kp, ko)
ss += '}'

print('''
#define VECTOR_STATIC_OUTSOURCE 1
#define VECTOR_STATIC_SIZE {}
#include <util/pstl/vector_static.h>
typedef wiselib::vector_static<Os, TupleT, VECTOR_STATIC_SIZE> PrecompiledTupleContainer;

/*
	#if VECTOR_STATIC_OUTSOURCE
		tuple_container_->set_data(tuple_data_);
		tuple_container_->set_size(VECTOR_STATIC_SIZE);
	#endif
*/

//Uint<sizeof(block_data_t*)>::t
TupleT::value_t tuple_data_[VECTOR_STATIC_SIZE * 3] = {};

'''.format(len(tuples), ss))



#for ks, kp, ko in tuples:
	#print(
			#find_dictionary(ks)['s'],
			#find_dictionary(kp)['s'],
			#find_dictionary(ko)['s'])

#  vim: set ts=4 sw=4 tw=78 noexpandtab :
