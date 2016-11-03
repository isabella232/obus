/******************************************************************************
 * libobus-java - obus client java binding library.
 *
 * Copyright (c) 2013 Parrot S.A.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Parrot Company nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PARROT COMPANY BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

package com.parrot.obus.internal;

import java.util.Arrays;
import java.util.Iterator;

import com.parrot.obus.internal.Core.DecodeError;
import com.parrot.obus.internal.Descriptor.FieldDesc;
import com.parrot.obus.internal.Descriptor.FieldDriverEnum;
import com.parrot.obus.internal.Descriptor.FieldType;
import com.parrot.obus.internal.Descriptor.StructDesc;


/**
 * Obus structure content.
 */
public class ObusStruct {
	private static final Core.Logger _log = Core.getLogger("obus");

	private final StructDesc desc; /**< Structure descriptor */
	private final Object[] fields; /**< Array of fields */
	private final int[] fieldsInt; /**< Array of fields for int */

	/**
	 * Create a new obus structure.
	 * @param structDesc : structure descriptor.
	 */
	private ObusStruct(StructDesc structDesc) {
		this.desc = structDesc;
		this.fields = new Object[structDesc.getFieldsDesc().size()];
		this.fieldsInt = new int[structDesc.getFieldsDesc().size()];
	}

	/**
	 * @return the desc
	 */
	public final StructDesc getDesc() {
		return desc;
	}

	/**
	 * Determine if a field is present (non null).
	 * @param fieldDesc : descriptor of field to query.
	 * @return true if the field is present, false otherwise.
	 */
	public final boolean hasField(FieldDesc fieldDesc) {
		return (this.fields[fieldDesc.getIdx()] != null);
	}

	/**
	 * Get a field that is a boolean.
	 * @param fieldDesc : descriptor of field to query.
	 * @return field.
	 */
	public final boolean getFieldBool(FieldDesc fieldDesc) {
		return (Boolean)this.fields[fieldDesc.getIdx()];
	}

	/**
	 * Get a field that is an array of boolean.
	 * @param fieldDesc : descriptor of field to query.
	 * @return field.
	 */
	public final boolean[] getFieldBoolArray(FieldDesc fieldDesc) {
		return (boolean[])this.fields[fieldDesc.getIdx()];
	}

	/**
	 * Get a field that is an int.
	 * @param fieldDesc : descriptor of field to query.
	 * @return field.
	 */
	public final int getFieldInt(FieldDesc fieldDesc) {
		return this.fieldsInt[fieldDesc.getIdx()];
	}

	/**
	 * Get a field that is an array of int.
	 * @param fieldDesc : descriptor of field to query.
	 * @return field.
	 */
	public final int[] getFieldIntArray(FieldDesc fieldDesc) {
		return (int[])this.fields[fieldDesc.getIdx()];
	}

	/**
	 * Get a field that is a long.
	 * @param fieldDesc : descriptor of field to query.
	 * @return field.
	 */
	public final Long getFieldLong(FieldDesc fieldDesc) {
		return (Long)this.fields[fieldDesc.getIdx()];
	}

	/**
	 * Get a field that is an array of long.
	 * @param fieldDesc : descriptor of field to query.
	 * @return field.
	 */
	public final Long[] getFieldLongArray(FieldDesc fieldDesc) {
		return (Long[])this.fields[fieldDesc.getIdx()];
	}

	/**
	 * Get a field that is an Enum.
	 * @param fieldDesc : descriptor of field to query.
	 * @return field.
	 */
	public final Enum<?> getFieldEnum(FieldDesc fieldDesc) {
		return (Enum<?>)this.fields[fieldDesc.getIdx()];
	}

	/**
	 * Get a field that is an array of Enum.
	 * @param fieldDesc : descriptor of field to query.
	 * @return field.
	 */
	public final Enum<?>[] getFieldEnumArray(FieldDesc fieldDesc) {
		return (Enum<?>[])this.fields[fieldDesc.getIdx()];
	}

	/**
	 * Get a field that is a String.
	 * @param fieldDesc : descriptor of field to query.
	 * @return field.
	 */
	public final String getFieldString(FieldDesc fieldDesc) {
		return (String)this.fields[fieldDesc.getIdx()];
	}

	/**
	 * Get a field that is an array of String.
	 * @param fieldDesc : descriptor of field to query.
	 * @return field.
	 */
	public final String[] getFieldStringArray(FieldDesc fieldDesc) {
		return (String[])this.fields[fieldDesc.getIdx()];
	}

	/**
	 * Set a field that is a boolean.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldBool(FieldDesc fieldDesc, boolean val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is an array of boolean.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldBoolArray(FieldDesc fieldDesc, boolean[] val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is an int.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldInt(FieldDesc fieldDesc, int val) {
		/* The presence of the field is indicated by a Boolean in this.fields */
		this.fields[fieldDesc.getIdx()] = Boolean.TRUE;
		this.fieldsInt[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is an array of int.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldIntArray(FieldDesc fieldDesc, int[] val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is a long.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldLong(FieldDesc fieldDesc, Long val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is an array of long.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldLongArray(FieldDesc fieldDesc, Long[] val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is an Enum.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldEnum(FieldDesc fieldDesc, Enum<?> val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is an array of Enum.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldEnumArray(FieldDesc fieldDesc, Enum<?>[] val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is a String.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldString(FieldDesc fieldDesc, String val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is an array of String.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldStringArray(FieldDesc fieldDesc, String[] val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is a float.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldFloat(FieldDesc fieldDesc, Float val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is an array of float.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldFloatArray(FieldDesc fieldDesc, Float[] val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is a double.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldDouble(FieldDesc fieldDesc, Double val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Set a field that is an array of double.
	 * @param fieldDesc : descriptor of field to set.
	 * @param val : value to set.
	 */
	public final void setFieldDoubleArray(FieldDesc fieldDesc, Double[] val) {
		this.fields[fieldDesc.getIdx()] = val;
	}

	/**
	 * Merge the content of this structure with another one.
	 * @param other : other structure to merge with this one.
	 */
	public final void merge(ObusStruct other) {
		if (this.desc != other.desc) {
			throw new IllegalStateException();
		}

		/* Merge fields */
		for (FieldDesc fieldDesc: this.desc.getFieldsDesc().values()) {
			int idx = fieldDesc.getIdx();
			if (other.hasField(fieldDesc)) {
				/* Direct copy */
				this.fields[idx] = other.fields[idx];
				this.fieldsInt[idx] = other.fieldsInt[idx];
			}
		}
	}

	/**
	 * Create a default empty structure.
	 * @param structDesc : structure descriptor.
	 */
	public static ObusStruct create(StructDesc structDesc) {
		/* Create ObusStruct object */
		return new ObusStruct(structDesc);
	}

	/**
	 * Determine if a field type is an integer.
	 * @param fieldType : field type.
	 * @return true if the field type is an integer, false otherwise.
	 */
	private static boolean isFieldTypeInt(int fieldType) {
		int fieldTypeBase = fieldType&FieldType.OBUS_FIELD_MASK;
		return (fieldTypeBase == FieldType.OBUS_FIELD_U8 ||
				fieldTypeBase == FieldType.OBUS_FIELD_I8 ||
				fieldTypeBase == FieldType.OBUS_FIELD_U16 ||
				fieldTypeBase == FieldType.OBUS_FIELD_I16 ||
				fieldTypeBase == FieldType.OBUS_FIELD_U32 ||
				fieldTypeBase == FieldType.OBUS_FIELD_I32);
	}

	/**
	 * Encode a primitive value that is not a integer.
	 * @param fieldDesc : descriptor of field to encode.
	 * @param buf : output buffer.
	 * @param val : value to encode.
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	private static void encodeValue(FieldDesc fieldDesc, Buffer buf, Object val) {
		switch (fieldDesc.type&FieldType.OBUS_FIELD_MASK) {
		case FieldType.OBUS_FIELD_U64:
			buf.writeU64((Long)val);
			break;

		case FieldType.OBUS_FIELD_I64:
			buf.writeI64((Long)val);
			break;

		case FieldType.OBUS_FIELD_STRING:
			buf.writeString((String)val);
			break;

		case FieldType.OBUS_FIELD_BOOL:
			buf.writeU8(Boolean.TRUE.equals(val) ? 1 : 0);
			break;

		case FieldType.OBUS_FIELD_ENUM:
			buf.writeI32(((FieldDriverEnum)fieldDesc.driver).toInt((Enum)val));
			break;

		case FieldType.OBUS_FIELD_F32:
			buf.writeF32((Float)val);
			break;

		case FieldType.OBUS_FIELD_F64:
			buf.writeF64((Double)val);
			break;

		default:
			/* This should not occur during encoding as all types are known */
			throw new IllegalArgumentException("ObusStruct.encodeValue: " +
					"unknown field type=" + fieldDesc.type);
		}
	}

	/**
	 * Encode a primitive value that is an integer.
	 * @param fieldDesc : descriptor of field to encode.
	 * @param buf : output buffer.
	 * @param val : value to encode.
	 */
	private static void encodeValueInt(FieldDesc fieldDesc, Buffer buf, int val) {
		switch (fieldDesc.type&FieldType.OBUS_FIELD_MASK) {
		case FieldType.OBUS_FIELD_U8:
			buf.writeU8(val);
			break;

		case FieldType.OBUS_FIELD_I8:
			buf.writeI8(val);
			break;

		case FieldType.OBUS_FIELD_U16:
			buf.writeU16(val);
			break;

		case FieldType.OBUS_FIELD_I16:
			buf.writeI16(val);
			break;

		case FieldType.OBUS_FIELD_U32:
			buf.writeU32(val);
			break;

		case FieldType.OBUS_FIELD_I32:
			buf.writeI32(val);
			break;

		default:
			/* This should not occur during encoding as all types are known */
			throw new IllegalArgumentException("ObusStruct.encodeValueInt: " +
					"unknown field type=" + fieldDesc.type);
		}
	}

	/**
	 * Encode a field that is an array.
	 * @param fieldDesc : descriptor of field to encode.
	 * @param buf : output buffer.
	 * @param array : array to  encode.
	 */
	private static void encodeArray(FieldDesc fieldDesc, Buffer buf, Object[] array) {
		/* Write number of items, then encode items */
		buf.writeU32(array.length);
		for (Object item: array) {
			ObusStruct.encodeValue(fieldDesc, buf, item);
		}
	}

	/**
	 * Encode a field that is an array of int.
	 * @param fieldDesc : descriptor of field to encode.
	 * @param buf : output buffer.
	 * @param array : array to  encode.
	 */
	private static void encodeArrayInt(FieldDesc fieldDesc, Buffer buf, int[] array) {
		/* Write number of items, then encode items */
		buf.writeU32(array.length);
		for (int item: array) {
			ObusStruct.encodeValueInt(fieldDesc, buf, item);
		}
	}

	/**
	 * Encode the structure.
	 * @param buf : output buffer.
	 */
	public final void encode(Buffer buf) {
		/* Get number of present fields */
		int fieldCount = 0;
		for (FieldDesc fieldDesc: this.desc.getFieldsDesc().values()) {
			if (this.hasField(fieldDesc)) {
				fieldCount++;
			}
		}

		/* Write number of fields, then encode fields */
		buf.writeU16(fieldCount);
		for (FieldDesc fieldDesc: this.desc.getFieldsDesc().values()) {
			int idx = fieldDesc.getIdx();
			if (this.hasField(fieldDesc)) {
				/* Write field uid and field type */
				buf.writeU16(fieldDesc.uid);
				buf.writeU8(fieldDesc.type);

				/* Encode field */
				if (ObusStruct.isFieldTypeInt(fieldDesc.type)) {
					if ((fieldDesc.type&FieldType.OBUS_FIELD_ARRAY) != 0) {
						int[] field = (int[])this.fields[idx];
						ObusStruct.encodeArrayInt(fieldDesc, buf, field);
					} else {
						int field = this.fieldsInt[idx];
						ObusStruct.encodeValueInt(fieldDesc, buf, field);
					}
				} else {
					if ((fieldDesc.type&FieldType.OBUS_FIELD_ARRAY) != 0) {
						Object[] field = (Object[])this.fields[idx];
						ObusStruct.encodeArray(fieldDesc, buf, field);
					} else {
						Object field = this.fields[idx];
						ObusStruct.encodeValue(fieldDesc, buf, field);
					}
				}
			}
		}
	}

	/**
	 * Decode a primitive value that is not a integer.
	 * @param fieldDesc : descriptor of field to decode.
	 * @param buf : input buffer.
	 * @return : decoded value.
	 * @throws DecodeError in case of error during decoding.
	 */
	@SuppressWarnings("rawtypes")
	private static Object decodeValue(FieldDesc fieldDesc, Buffer buf)
			throws DecodeError {
		switch (fieldDesc.type&FieldType.OBUS_FIELD_MASK) {
		case FieldType.OBUS_FIELD_U64:
			return buf.readU64();

		case FieldType.OBUS_FIELD_I64:
			return buf.readI64();

		case FieldType.OBUS_FIELD_STRING:
			return buf.readString();

		case FieldType.OBUS_FIELD_BOOL:
			return buf.readU8() == 1;

		case FieldType.OBUS_FIELD_ENUM:
			return ((FieldDriverEnum)fieldDesc.driver).fromInt(buf.readI32());

		case FieldType.OBUS_FIELD_F32:
			return buf.readF32();

		case FieldType.OBUS_FIELD_F64:
			return buf.readF64();

		default:
			throw new DecodeError("ObusStruct.decodeValue: " +
					"unknown field type=%d" + fieldDesc.type);
		}
	}

	/**
	 * Decode a primitive value that is an integer.
	 * @param fieldDesc : descriptor of field to decode.
	 * @param buf : input buffer.
	 * @return : decoded value.
	 * @throws DecodeError in case of error during decoding.
	 */
	private static int decodeValueInt(FieldDesc fieldDesc, Buffer buf)
			throws DecodeError {
		switch (fieldDesc.type&FieldType.OBUS_FIELD_MASK) {
		case FieldType.OBUS_FIELD_U8:
			return buf.readU8();

		case FieldType.OBUS_FIELD_I8:
			return buf.readI8();

		case FieldType.OBUS_FIELD_U16:
			return buf.readU16();

		case FieldType.OBUS_FIELD_I16:
			return buf.readI16();

		case FieldType.OBUS_FIELD_U32:
			return buf.readU32();

		case FieldType.OBUS_FIELD_I32:
			return buf.readI32();

		default:
			throw new DecodeError("ObusStruct.decodeValueInt: " +
					"unknown field type=%d" + fieldDesc.type);
		}
	}

	/**
	 * Decode a field that is an array.
	 * @param fieldDesc : descriptor of field to decode.
	 * @param buf : input buffer.
	 * @return decoded array as a simple Object.
	 * @throws DecodeError in case of error during decoding.
	 */
	private static Object decodeArray(FieldDesc fieldDesc, Buffer buf)
			throws DecodeError {

		/* Read number of items */
		int itemCount = buf.readU32();

		/* Create array of correct type */
		Object[] array = null;
		switch (fieldDesc.type&FieldType.OBUS_FIELD_MASK) {
		case FieldType.OBUS_FIELD_U64:
			array = new Long[itemCount];
			break;

		case FieldType.OBUS_FIELD_I64:
			array = new Long[itemCount];
			break;

			case FieldType.OBUS_FIELD_ENUM:
			array = (Object[])java.lang.reflect.Array.newInstance(
				fieldDesc.driver.cls, itemCount);
			break;

		case FieldType.OBUS_FIELD_STRING:
			array = new String[itemCount];
			break;

		case FieldType.OBUS_FIELD_BOOL:
			array = new Boolean[itemCount];
			break;

		case FieldType.OBUS_FIELD_F32:
			array = new Float[itemCount];
			break;

		case FieldType.OBUS_FIELD_F64:
			array = new Double[itemCount];
			break;

		default:
			throw new DecodeError("ObusStruct.decodeArray: " +
					"unknown field type=%d" + fieldDesc.type);
		}

		/* Decode items */
		for (int i = 0; i < itemCount; i++) {
			array[i] = ObusStruct.decodeValue(fieldDesc, buf);
		}

		/* Return array as a single object... */
		return array;
	}

	/**
	 * Decode a field that is an array of int.
	 * @param fieldDesc : descriptor of field to decode.
	 * @param buf : input buffer.
	 * @return decoded array as a simple Object.
	 * @throws DecodeError in case of error during decoding.
	 */
	private static Object decodeArrayInt(FieldDesc fieldDesc, Buffer buf)
			throws DecodeError {

		/* Read number of items */
		int itemCount = buf.readU32();

		/* Decode items */
		int[] array = new int[itemCount];
		for (int i = 0; i < itemCount; i++) {
			array[i] = ObusStruct.decodeValueInt(fieldDesc, buf);
		}

		/* Return array as a single object... */
		return array;
	}

	/**
	 * Skip a field.
	 * @param fieldType : field type to skip.
	 * @throws DecodeError in case of error during decoding.
	 */
	private static void skipField(int fieldType, Buffer buf) throws DecodeError {
		switch (fieldType&FieldType.OBUS_FIELD_MASK) {
		case FieldType.OBUS_FIELD_U8:
		case FieldType.OBUS_FIELD_BOOL:
			buf.readU8();
			break;

		case FieldType.OBUS_FIELD_I8:
			buf.readI8();
			break;

		case FieldType.OBUS_FIELD_U16:
			buf.readU16();
			break;

		case FieldType.OBUS_FIELD_I16:
			buf.readI16();
			break;

		case FieldType.OBUS_FIELD_U32:
			buf.readU32();
			break;

		case FieldType.OBUS_FIELD_I32:
			buf.readI32();
			break;

		case FieldType.OBUS_FIELD_U64:
			buf.readU64();
			break;

		case FieldType.OBUS_FIELD_I64:
			buf.readI64();
			break;

		case FieldType.OBUS_FIELD_STRING:
			buf.readString();
			break;

		case FieldType.OBUS_FIELD_ENUM:
			buf.readI32();
			break;

		case FieldType.OBUS_FIELD_F32:
			buf.readF32();
			break;

		case FieldType.OBUS_FIELD_F64:
			buf.readF64();
			break;

		default:
			throw new DecodeError("ObusStruct.skipField: " +
					"unknown field type=%d" + fieldType);
		}
	}

	/**
	 * Skip a field that is an array.
	 * @param fieldType : field type to skip.
	 * @throws DecodeError in case of error during decoding.
	 */
	private static void skipFieldArray(int fieldType, Buffer buf) throws DecodeError {
		/* Read number of items, then skip items */
		int itemCount = buf.readU32();
		for (int i = 0; i < itemCount; i++) {
			ObusStruct.skipField(fieldType, buf);
		}
	}

	/**
	 * Decode a structure.
	 * @param structDesc : structure descriptor.
	 * @param buf : input buffer.
	 * @return decoded structure.
	 * @throws DecodeError in case of error during decoding.
	 */
	public static ObusStruct decode(StructDesc structDesc, Buffer buf)
			throws DecodeError {
		/* Create ObusStruct */
		ObusStruct struct = ObusStruct.create(structDesc);

		/* Read number of fields */
		int fieldCount = buf.readU16();
		for (int i = 0; i < fieldCount; i++) {
			/* Read field uid and field type */
			int uid = buf.readU16();
			int fieldType = buf.readU8();

			/* Get field descriptor from uid */
			FieldDesc fieldDesc = structDesc.getFieldsDesc().get(uid);
			if (fieldDesc == null) {
				/* Log and try to skip field */
				_log.warning("ObusStruct.decode: " +
						"Can't decode field uid=" + uid + ", descriptor not found");
				if ((fieldType&FieldType.OBUS_FIELD_ARRAY) != 0) {
					ObusStruct.skipFieldArray(fieldType, buf);
				} else {
					ObusStruct.skipField(fieldType, buf);
				}
			} else if(fieldType != fieldDesc.type) {
				/* Log and try to skip field TODO: */
				_log.warning("ObusStruct.decode: " +
						"Can't decode field uid=" + uid +
						", type mismatch (descriptor:" + fieldDesc.type +
						" decoded="+ fieldType + ")");
				if ((fieldType&FieldType.OBUS_FIELD_ARRAY) != 0) {
					ObusStruct.skipFieldArray(fieldType, buf);
				} else {
					ObusStruct.skipField(fieldType, buf);
				}
			} else {
				/* Decode field */
				if (ObusStruct.isFieldTypeInt(fieldDesc.type)) {
					if ((fieldDesc.type&FieldType.OBUS_FIELD_ARRAY) != 0) {
						Object field = ObusStruct.decodeArrayInt(fieldDesc, buf);
						struct.fields[fieldDesc.getIdx()] = field;
					} else {
						int field = ObusStruct.decodeValueInt(fieldDesc, buf);
						struct.fields[fieldDesc.getIdx()] = Boolean.TRUE;
						struct.fieldsInt[fieldDesc.getIdx()] = field;
					}
				} else {
					if ((fieldDesc.type&FieldType.OBUS_FIELD_ARRAY) != 0) {
						Object field = ObusStruct.decodeArray(fieldDesc, buf);
						struct.fields[fieldDesc.getIdx()] = field;
					} else {
						Object field = ObusStruct.decodeValue(fieldDesc, buf);
						struct.fields[fieldDesc.getIdx()] = field;
					}
				}
			}
		}

		return struct;
	}

	/**
	 * Convert structure to a String.
	 * @return string representation of structure.
	 */
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append('{');
		Iterator<FieldDesc> it = this.desc.getFieldsDesc().values().iterator();
		boolean first = true;
		while (it.hasNext()) {
			FieldDesc fieldDesc = it.next();
			if (this.hasField(fieldDesc)) {
				int idx = fieldDesc.getIdx();
				if (!first) {
					sb.append(", ");
				}
				sb.append(fieldDesc.name);
				sb.append('=');
				if (ObusStruct.isFieldTypeInt(fieldDesc.type)) {
					if ((fieldDesc.type&FieldType.OBUS_FIELD_ARRAY) != 0) {
						sb.append(Arrays.toString((int[])this.fields[idx]));
					} else {
						sb.append(this.fieldsInt[idx]);
					}
				} else {
					if ((fieldDesc.type&FieldType.OBUS_FIELD_ARRAY) != 0) {
						sb.append(Arrays.toString((Object[])this.fields[idx]));
					} else {
						sb.append(this.fields[idx]);
					}
				}
				first = false;
			}
		}
		sb.append('}');
		return sb.toString();
	}
}
