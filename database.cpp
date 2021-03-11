/*
* Covariant Script ODBC support
*
* Licensed under the Covariant General Public License,
* Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* https://covariant.cn/licenses/LICENSE-1.1
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Copyright (C) 2020 Michael Lee(李登淳)
* Email: mikecovlee@163.com
* Github: https://github.com/mikecovlee
*/
#include "nanodbc/nanodbc.h"

#include <covscript/dll.hpp>
#include <covscript/cni.hpp>
#include <codecvt>

#define HANDLE_EXCEPTION(expr) try { expr; } \
    catch (const nanodbc::type_incompatible_error& e) { throw cs::lang_error(std::string("cs::odbc::type_incompatible_error: ") + e.what()); } \
    catch (const nanodbc::null_access_error& e)       { throw cs::lang_error(std::string("cs::odbc::null_access_error: ") + e.what()); }       \
    catch (const nanodbc::index_range_error& e)       { throw cs::lang_error(std::string("cs::odbc::index_range_error: ") + e.what()); }       \
    catch (const nanodbc::database_error& e)          { throw cs::lang_error(std::string("cs::odbc::database_error: ") + e.what()); }

#ifdef NANODBC_ENABLE_UNICODE

std::string _wide2local(const nanodbc::string &wide)
{
	using char_t = nanodbc::string::value_type;
	return std::wstring_convert<std::codecvt_utf8<char_t>, char_t> {}.to_bytes(wide);
}

nanodbc::string _local2wide(const std::string &local)
{
	using char_t = nanodbc::string::value_type;
	return std::wstring_convert<std::codecvt_utf8<char_t>, char_t> {}.from_bytes(local);
}

#define wide2local(str) _wide2local(str)
#define local2wide(str) _local2wide(str)
#else
#define wide2local(str) str
#define local2wide(str) str
#endif

CNI_ROOT_NAMESPACE {
	cs::var list_drivers()
	{
		auto lst = nanodbc::list_drivers();
		cs::var ret = cs::var::make<cs::array>();
		cs::array &arr = ret.val<cs::array>();
		for (auto &it:lst)
			arr.push_back(it.name);
		return ret;
	}

	CNI(list_drivers)

	cs::var list_datasources()
	{
		auto lst = nanodbc::list_datasources();
		cs::var ret = cs::var::make<cs::array>();
		cs::array &arr = ret.val<cs::array>();
		for (auto &it:lst)
			arr.push_back(cs::pair(it.name, it.driver));
		return ret;
	}

	CNI(list_datasources)

	nanodbc::connection connect(const cs::string &str)
	{
		HANDLE_EXCEPTION(return nanodbc::connection(local2wide(str)))
	}

	CNI(connect)

	nanodbc::connection connect_ds(const std::string &dsn, const std::string &user, const std::string &pass)
	{
		HANDLE_EXCEPTION(return nanodbc::connection(local2wide(dsn), local2wide(user), local2wide(pass)))
	}

	CNI(connect_ds)

	CNI_NAMESPACE(connection)
	{
		CNI_V(connected, &nanodbc::connection::connected)
		CNI_V(disconnect, &nanodbc::connection::disconnect)
		CNI_V(dbms_name, &nanodbc::connection::dbms_name)
		CNI_V(dbms_version, &nanodbc::connection::dbms_version)
		CNI_V(driver_name, &nanodbc::connection::driver_name)
		CNI_V(database_name, &nanodbc::connection::database_name)

		nanodbc::result execute(nanodbc::connection &conn, const std::string &query) {
			HANDLE_EXCEPTION(return nanodbc::execute(conn, local2wide(query)))
		}

		CNI(execute)

		void just_execute(nanodbc::connection &conn, const std::string &query) {
			HANDLE_EXCEPTION(nanodbc::just_execute(conn, local2wide(query)))
		}

		CNI(just_execute)

		nanodbc::statement prepare(nanodbc::connection &conn, const std::string &query) {
			HANDLE_EXCEPTION(return nanodbc::statement(conn, local2wide(query)))
		}

		CNI(prepare)
	}

	CNI_NAMESPACE(result)
	{
		CNI_V(columns, &nanodbc::result::columns)

		bool next(nanodbc::result &r) {
			HANDLE_EXCEPTION(return r.next())
		}

		CNI(next)

		cs::number get_integer(const nanodbc::result &r, short column) {
			HANDLE_EXCEPTION(return r.get<long>(column))
		}

		CNI(get_integer)

		cs::number get_real(const nanodbc::result &r, short column) {
			HANDLE_EXCEPTION(return r.get<double>(column))
		}

		CNI(get_real)

		std::string get_string(const nanodbc::result &r, short column) {
			HANDLE_EXCEPTION(return wide2local(r.get<nanodbc::string>(column)))
		}

		CNI(get_string)

		std::string column_name(const nanodbc::result &r, short column) {
			HANDLE_EXCEPTION(return wide2local(r.column_name(column)))
		}

		CNI(column_name)

		std::string column_datatype_name(const nanodbc::result &r, short column) {
			HANDLE_EXCEPTION(return wide2local(r.column_datatype_name(column)))
		}

		CNI(column_datatype_name)

		bool is_null(const nanodbc::result &r, short column) {
			return r.is_null(column);
		}

		CNI(is_null)
	}

	CNI_NAMESPACE(statement)
	{
		nanodbc::result execute(nanodbc::statement &stmt) {
			HANDLE_EXCEPTION(return stmt.execute())
		}

		CNI(execute)

		void just_execute(nanodbc::statement &stmt) {
			HANDLE_EXCEPTION(stmt.just_execute())
		}

		CNI(just_execute)

		CNI_V(reset_parameters, &nanodbc::statement::reset_parameters)
		CNI_V(parameters, &nanodbc::statement::parameters)

		/* TODO: Could not work properly
		void bind_integer(nanodbc::statement &stmt, short idx, long val)
		{
		    stmt.bind(idx, &val);
		}

		CNI(bind_integer)

		void bind_real(nanodbc::statement &stmt, short idx, double val)
		{
		    stmt.bind(idx, &val);
		}

		CNI(bind_real)
		*/

		void bind(nanodbc::statement &stmt, short idx, const std::string &val) {
			stmt.bind(idx, val.c_str());
		}

		CNI(bind)

		void bind_null(nanodbc::statement &stmt, short idx) {
			stmt.bind_null(idx);
		}

		CNI(bind_null)
	}
}

CNI_ENABLE_TYPE_EXT_V(connection, nanodbc::connection, cs::odbc::connection)
CNI_ENABLE_TYPE_EXT_V(result, nanodbc::result, cs::odbc::result)
CNI_ENABLE_TYPE_EXT_V(statement, nanodbc::statement, cs::odbc::statement)
