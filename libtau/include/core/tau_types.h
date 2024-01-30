#pragma once

#include "core.h"

#include <unordered_map>

namespace tau {
	typedef u64 _type_id;

	constexpr _type_id TEMPLATE_TYPE_BASE = 0x1000000000000000;

	struct FieldDef {
		std::string name;
		size_t offset;
		_type_id type;
	};

	class TypeRegistry;

	struct TypeID {
		friend class TypeRegistry;

		_type_id id;
		bool is_user_defined;
		std::string true_name;
		size_t size;
		
		std::vector<FieldDef> fields;

	private:
		void calculate_size_and_offsets(TypeRegistry&);
	};

	class TypeRegistry {
	private:
		TypeRegistry();

	public:
		static TypeRegistry& instance();

		TypeRegistry(const TypeRegistry&) = delete;

		_type_id get_id_from_name(std::string_view name);
		
		size_t size_of(_type_id id);
		std::string name_of(_type_id id);
		std::vector<FieldDef>& fields_of(_type_id id);
		size_t offset_of(_type_id id, const std::string& field_name);

		bool is_struct(_type_id id);
		_type_id get_struct_field_type(_type_id struct_type, const std::string& field_name);

		result<_type_id> define_type(const std::string& type_name, size_t size);
		result<_type_id> define_type(const std::string& type_name, std::vector<FieldDef>& fields);


	private:
		std::unordered_map<_type_id, TypeID> m_Types;
	};


}

