# ***************************************************************************
# ** Copyright (c) 2005 Sergey Ryazanov (http://home.onego.ru/~ryazanov)   **
# **                                                                       **
# ** Permission is hereby granted, free of charge, to any person           **
# ** obtaining a copy of this software and associated documentation        **
# ** files (the "Software"), to deal in the Software without               **
# ** restriction, including without limitation the rights to use, copy,    **
# ** modify, merge, publish, distribute, sublicense, and/or sell copies    **
# ** of the Software, and to permit persons to whom the Software is        **
# ** furnished to do so, subject to the following conditions:              **
# **                                                                       **
# ** The above copyright notice and this permission notice shall be        **
# ** included in all copies or substantial portions of the Software.       **
# **                                                                       **
# ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       **
# ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    **
# ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 **
# ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT           **
# ** HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,          **
# ** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,    **
# ** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         **
# ** DEALINGS IN THE SOFTWARE.                                             **
# ***************************************************************************/
#
#/***************************************************************************
# ** As given in
# **   http://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
# ** this file is under the MIT license, see
# **   http://www.opensource.org/licenses/mit-license.php
# ** for details.
# **
# ** Kind regards,
# ** Tobias Baumgartner
# **
# ***************************************************************************/

#/usr/bin/perl
use strict;

#configuration
my $max_param_count = 10;

#implementation
open(OUTPUT, ">delegate_list.hpp") || die $!;
print(OUTPUT "// Automaticly generaged by $0\n\n");

my @param_list;
for (my $param_count = 0; $param_count <= $max_param_count;
		push(@param_list, ++$param_count))
{
	print(OUTPUT "// $param_count params\n");

	print(OUTPUT "#define SRUTIL_DELEGATE_PARAM_COUNT $param_count\n");
	
	print(OUTPUT "#define SRUTIL_DELEGATE_TEMPLATE_PARAMS ",
		join(", ", map({"typename A$_"} @param_list)), "\n");

	print(OUTPUT "#define SRUTIL_DELEGATE_TEMPLATE_ARGS ",
		join(", ", map({"A$_"} @param_list)), "\n");

	print(OUTPUT "#define SRUTIL_DELEGATE_PARAMS ",
		join(", ", map({"A$_ a$_"} @param_list)), "\n");

	print(OUTPUT "#define SRUTIL_DELEGATE_ARGS ",
		join(",", map({"a$_"} @param_list)), "\n");

	print(OUTPUT "#define SRUTIL_DELEGATE_INVOKER_INITIALIZATION_LIST ",
		join(",", map({"a$_(a$_)"} @param_list)), "\n");

	print(OUTPUT "#define SRUTIL_DELEGATE_INVOKER_DATA ",
		map({"A$_ a$_;"} @param_list), "\n");

	print(OUTPUT '#include "delegate_template.hpp"', "\n",
		"#undef SRUTIL_DELEGATE_PARAM_COUNT\n",
		"#undef SRUTIL_DELEGATE_TEMPLATE_PARAMS\n",
		"#undef SRUTIL_DELEGATE_TEMPLATE_ARGS\n",
		"#undef SRUTIL_DELEGATE_PARAMS\n",
		"#undef SRUTIL_DELEGATE_ARGS\n",
		"#undef SRUTIL_DELEGATE_INVOKER_INITIALIZATION_LIST\n",
		"#undef SRUTIL_DELEGATE_INVOKER_DATA\n",
		"\n");
}
close(OUTPUT) || die $!;
0;
