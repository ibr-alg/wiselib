/***************************************************************************
 ** Copyright (c) 2005 Sergey Ryazanov (http://home.onego.ru/~ryazanov)   **
 **                                                                       **
 ** Permission is hereby granted, free of charge, to any person           **
 ** obtaining a copy of this software and associated documentation        **
 ** files (the "Software"), to deal in the Software without               **
 ** restriction, including without limitation the rights to use, copy,    **
 ** modify, merge, publish, distribute, sublicense, and/or sell copies    **
 ** of the Software, and to permit persons to whom the Software is        **
 ** furnished to do so, subject to the following conditions:              **
 **                                                                       **
 ** The above copyright notice and this permission notice shall be        **
 ** included in all copies or substantial portions of the Software.       **
 **                                                                       **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       **
 ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    **
 ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 **
 ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT           **
 ** HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,          **
 ** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,    **
 ** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         **
 ** DEALINGS IN THE SOFTWARE.                                             **
 ***************************************************************************/

/***************************************************************************
 ** As given in
 **   http://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
 ** this file is under the MIT license, see
 **   http://www.opensource.org/licenses/mit-license.php
 ** for details.
 **
 ** Kind regards,
 ** Tobias Baumgartner
 **
 ***************************************************************************/
#ifndef SRUTIL_DELEGATE_INCLUDED
#define SRUTIL_DELEGATE_INCLUDED

// namespace srutil
// {
#ifdef SRUTIL_DELEGATE_PREFERRED_SYNTAX
	template <typename TSignature> class delegate;
	template <typename TSignature> class delegate_invoker;
#endif
// }

#ifdef _MSC_VER
#define SRUTIL_DELEGATE_CALLTYPE __fastcall
#else
#define SRUTIL_DELEGATE_CALLTYPE
#endif

#include "detail/delegate_list.hpp"

#undef SRUTIL_DELEGATE_CALLTYPE

#endif//SRUTIL_DELEGATE_INCLUDED
