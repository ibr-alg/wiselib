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
#if SRUTIL_DELEGATE_PARAM_COUNT > 0
#define SRUTIL_DELEGATE_SEPARATOR ,
#else
#define SRUTIL_DELEGATE_SEPARATOR
#endif

// see BOOST_JOIN for explanation
#define SRUTIL_DELEGATE_JOIN_MACRO( X, Y) SRUTIL_DELEGATE_DO_JOIN( X, Y )
#define SRUTIL_DELEGATE_DO_JOIN( X, Y ) SRUTIL_DELEGATE_DO_JOIN2(X,Y)
#define SRUTIL_DELEGATE_DO_JOIN2( X, Y ) X##Y

// namespace srutil
// {
#ifdef SRUTIL_DELEGATE_PREFERRED_SYNTAX
#define SRUTIL_DELEGATE_CLASS_NAME delegate
#define SRUTIL_DELEGATE_INVOKER_CLASS_NAME delegate_invoker
#else
#define SRUTIL_DELEGATE_CLASS_NAME SRUTIL_DELEGATE_JOIN_MACRO(delegate,SRUTIL_DELEGATE_PARAM_COUNT)
#define SRUTIL_DELEGATE_INVOKER_CLASS_NAME SRUTIL_DELEGATE_JOIN_MACRO(delegate_invoker,SRUTIL_DELEGATE_PARAM_COUNT)
	template <typename R SRUTIL_DELEGATE_SEPARATOR SRUTIL_DELEGATE_TEMPLATE_PARAMS>
	class SRUTIL_DELEGATE_INVOKER_CLASS_NAME;
#endif

	template <typename R SRUTIL_DELEGATE_SEPARATOR SRUTIL_DELEGATE_TEMPLATE_PARAMS>
#ifdef SRUTIL_DELEGATE_PREFERRED_SYNTAX
	class SRUTIL_DELEGATE_CLASS_NAME<R (SRUTIL_DELEGATE_TEMPLATE_ARGS)>
#else
	class SRUTIL_DELEGATE_CLASS_NAME
#endif
	{
	public:
		typedef R return_type;
#ifdef SRUTIL_DELEGATE_PREFERRED_SYNTAX
		typedef return_type (SRUTIL_DELEGATE_CALLTYPE *signature_type)(SRUTIL_DELEGATE_TEMPLATE_ARGS);
		typedef SRUTIL_DELEGATE_INVOKER_CLASS_NAME<signature_type> invoker_type;
#else
      typedef return_type (SRUTIL_DELEGATE_CALLTYPE *signature_type)(SRUTIL_DELEGATE_TEMPLATE_ARGS);
		typedef SRUTIL_DELEGATE_INVOKER_CLASS_NAME<R SRUTIL_DELEGATE_SEPARATOR SRUTIL_DELEGATE_TEMPLATE_ARGS> invoker_type;
#endif

		SRUTIL_DELEGATE_CLASS_NAME()
			: object_ptr(0)
			, stub_ptr(0)
		{}

		template <return_type (*TMethod)(SRUTIL_DELEGATE_TEMPLATE_ARGS)>
		static SRUTIL_DELEGATE_CLASS_NAME from_function()
		{
			return from_stub(0, &function_stub<TMethod>);
		}

		template <class T, return_type (T::*TMethod)(SRUTIL_DELEGATE_TEMPLATE_ARGS)>
		static SRUTIL_DELEGATE_CLASS_NAME from_method(T* object_ptr)
		{
			return from_stub(object_ptr, &method_stub<T, TMethod>);
		}

		template <class T, return_type (T::*TMethod)(SRUTIL_DELEGATE_TEMPLATE_ARGS) const>
		static SRUTIL_DELEGATE_CLASS_NAME from_const_method(T const* object_ptr)
		{
			return from_stub(const_cast<T*>(object_ptr), &const_method_stub<T, TMethod>);
		}

		return_type operator()(SRUTIL_DELEGATE_PARAMS) const
		{
			return (*stub_ptr)(object_ptr SRUTIL_DELEGATE_SEPARATOR SRUTIL_DELEGATE_ARGS);
		}

		operator bool () const
		{
			return stub_ptr != 0;
		}

		bool operator!() const
		{
			return !(operator bool());
		}

      void* obj_ptr() { return object_ptr; };

	//private:
		
		typedef return_type (SRUTIL_DELEGATE_CALLTYPE *stub_type)(void* object_ptr SRUTIL_DELEGATE_SEPARATOR SRUTIL_DELEGATE_PARAMS);

		void* object_ptr;
		stub_type stub_ptr;

		static SRUTIL_DELEGATE_CLASS_NAME from_stub(void* object_ptr, stub_type stub_ptr)
		{
			SRUTIL_DELEGATE_CLASS_NAME d;
			d.object_ptr = object_ptr;
			d.stub_ptr = stub_ptr;
			return d;
		}

		template <return_type (*TMethod)(SRUTIL_DELEGATE_TEMPLATE_ARGS)>
		static return_type SRUTIL_DELEGATE_CALLTYPE function_stub(void* SRUTIL_DELEGATE_SEPARATOR SRUTIL_DELEGATE_PARAMS)
		{
			return (TMethod)(SRUTIL_DELEGATE_ARGS);
		}

		template <class T, return_type (T::*TMethod)(SRUTIL_DELEGATE_TEMPLATE_ARGS)>
		static return_type SRUTIL_DELEGATE_CALLTYPE method_stub(void* object_ptr SRUTIL_DELEGATE_SEPARATOR SRUTIL_DELEGATE_PARAMS)
		{
			T* p = static_cast<T*>(object_ptr);
			return (p->*TMethod)(SRUTIL_DELEGATE_ARGS);
		}

		template <class T, return_type (T::*TMethod)(SRUTIL_DELEGATE_TEMPLATE_ARGS) const>
		static return_type SRUTIL_DELEGATE_CALLTYPE const_method_stub(void* object_ptr SRUTIL_DELEGATE_SEPARATOR SRUTIL_DELEGATE_PARAMS)
		{
			T const* p = static_cast<T*>(object_ptr);
			return (p->*TMethod)(SRUTIL_DELEGATE_ARGS);
		}
	};

	template <typename R SRUTIL_DELEGATE_SEPARATOR SRUTIL_DELEGATE_TEMPLATE_PARAMS>
#ifdef SRUTIL_DELEGATE_PREFERRED_SYNTAX
	class SRUTIL_DELEGATE_INVOKER_CLASS_NAME<R (SRUTIL_DELEGATE_TEMPLATE_ARGS)>
#else
	class SRUTIL_DELEGATE_INVOKER_CLASS_NAME
#endif
	{
		SRUTIL_DELEGATE_INVOKER_DATA

	public:
		SRUTIL_DELEGATE_INVOKER_CLASS_NAME(SRUTIL_DELEGATE_PARAMS)
#if SRUTIL_DELEGATE_PARAM_COUNT > 0
			:
#endif
			SRUTIL_DELEGATE_INVOKER_INITIALIZATION_LIST
		{
		}

		template <class TDelegate>
		R operator()(TDelegate d) const
		{
			return d(SRUTIL_DELEGATE_ARGS);
		}
	};
// }

#undef SRUTIL_DELEGATE_CLASS_NAME
#undef SRUTIL_DELEGATE_SEPARATOR
#undef SRUTIL_DELEGATE_JOIN_MACRO
#undef SRUTIL_DELEGATE_DO_JOIN
#undef SRUTIL_DELEGATE_DO_JOIN2
