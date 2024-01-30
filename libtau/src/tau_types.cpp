#include "core/tau_types.h"

namespace tau {

	static _type_id s_ID_COUNTER = 0;

	TypeRegistry& TypeRegistry::instance() {
		static TypeRegistry registry;
		return registry;
	}

	void TypeID::calculate_size_and_offsets(TypeRegistry& registry) {

		size_t size = 0;
		size_t offset = 0;

		for (size_t i = 0; i < fields.size(); i++) {
			auto& field = fields[i];
			size_t size = registry.size_of(field.type);

			field.offset = offset;
			
			offset += size;
			size += size;
		}

		this->size = size;
		//TODO: take padding into consideration
	}

#define PRIMITIVE(name, size_b) \
		TypeID name ## _ty; \
		name ## _ty.true_name = #name; \
		name ## _ty.size = size_b; \
		name ## _ty.is_user_defined = false; \
		name ## _ty.id = ++s_ID_COUNTER; \
		m_Types[name ## _ty.id] = name ## _ty;

	TypeRegistry::TypeRegistry(){

		PRIMITIVE(void, 0);
		PRIMITIVE(u8, 1);
		PRIMITIVE(u16, 2);
		PRIMITIVE(u32, 4);
		PRIMITIVE(u64, 8);
		PRIMITIVE(i8, 1);
		PRIMITIVE(i16, 2);
		PRIMITIVE(i32, 4);
		PRIMITIVE(i64, 8);
		PRIMITIVE(f32, 4);
		PRIMITIVE(f64, 8);
		PRIMITIVE(char, 1);
		PRIMITIVE(bool, sizeof(bool));

		TypeID STATIC_INT_TY;
		STATIC_INT_TY.true_name = "long long";
		STATIC_INT_TY.size = sizeof(long long);
		STATIC_INT_TY.is_user_defined = false;
		STATIC_INT_TY.id = ++s_ID_COUNTER;
		m_Types[STATIC_INT_TY.id] = STATIC_INT_TY;

		TypeID STATIC_FLOAT_TY;
		STATIC_FLOAT_TY.true_name = "double";
		STATIC_FLOAT_TY.size = sizeof(double);
		STATIC_FLOAT_TY.is_user_defined = false;
		STATIC_FLOAT_TY.id = ++s_ID_COUNTER;
		m_Types[STATIC_FLOAT_TY.id] = STATIC_FLOAT_TY;
		// TODO: string?
	}

	_type_id TypeRegistry::get_id_from_name(std::string_view name) {
		for (auto& kv : m_Types) {
			if (kv.second.true_name == name) {
				return kv.first;
			}
		}

		return 0;
	}

	size_t TypeRegistry::size_of(_type_id id) {
		if (m_Types.find(id) == m_Types.end()) return 0;
		return m_Types.at(id).size;
	}

	std::string TypeRegistry::name_of(_type_id id) {
		if (m_Types.find(id) == m_Types.end()) return "Undefined";
		return (m_Types.at(id).is_user_defined ? "struct " : "") + m_Types.at(id).true_name;
	}

	std::vector<FieldDef>& TypeRegistry::fields_of(_type_id id) {
		return m_Types.at(id).fields;
	}

	size_t TypeRegistry::offset_of(_type_id id, const std::string& field_name) {
		auto& type = m_Types.at(id);

		for (size_t i = 0; i < type.fields.size(); i++) {
			if (type.fields[i].name == field_name) {
				return type.fields[i].offset;
			}
		}

		return 0;
	}

	_type_id TypeRegistry::get_struct_field_type(_type_id struct_type, const std::string& field_name) {
		if (m_Types.find(struct_type) == m_Types.end()) return 0;

		auto& type = m_Types.at(struct_type);
		for (auto& field : type.fields) {
			if (field.name == field_name) {
				return field.type;
			}
		}
		return 0;
	}

	bool TypeRegistry::is_struct(_type_id id) {
		if (m_Types.find(id) == m_Types.end()) return false;
		return m_Types.at(id).is_user_defined;
	}

	result<_type_id> TypeRegistry::define_type(const std::string& type_name, size_t size) {
		if (get_id_from_name(type_name.c_str()) != 0) {
			return result<_type_id>::Err("Type " + type_name + " is already defined");
		}

		TypeID type;
		type.true_name = type_name;
		type.size = size;
		type.is_user_defined = true;
		type.id = ++s_ID_COUNTER;

		m_Types[type.id] = type;

		return result<_type_id>::Ok(type.id);
	}

	result<_type_id> TypeRegistry::define_type(const std::string& type_name, std::vector<FieldDef>& fields) {
		if (get_id_from_name(type_name.c_str()) != 0) {
			return result<_type_id>::Err("Type " + type_name + " is already defined");
		}

		TypeID type;
		type.id = ++s_ID_COUNTER;
		type.is_user_defined = true;
		type.true_name = type_name;
		type.fields = fields;

		type.calculate_size_and_offsets(*this);

		m_Types[type.id] = type;

		return result<_type_id>::Ok(type.id);
	}

}