/** 
 * @file llvolumemessage.cpp
 * @brief LLVolumeMessage base class
 *
 * Copyright (c) 2001-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

#include "linden_common.h"

#include "message.h"
#include "llvolumemessage.h"
#include "lldatapacker.h"

//============================================================================

// LLVolumeMessage is just a wrapper class; all members are static

//============================================================================

bool LLVolumeMessage::packProfileParams(
	const LLProfileParams* params,
	LLMessageSystem *mesgsys)
{
	// Default to cylinder
	static LLProfileParams defaultparams(LL_PCODE_PROFILE_CIRCLE, U8(0), U8(0), U8(0));
	
	if (!params)
		params = &defaultparams;
	
	U8 tempU8;
	tempU8 = params->getCurveType();
	mesgsys->addU8Fast(_PREHASH_ProfileCurve, tempU8);

	tempU8 = (U8) llround( params->getBegin() / CUT_QUANTA);
	mesgsys->addU8Fast(_PREHASH_ProfileBegin, tempU8);

	tempU8 = 200 - (U8) llround(params->getEnd() / CUT_QUANTA);
	mesgsys->addU8Fast(_PREHASH_ProfileEnd, tempU8);

	tempU8 = (S8) llround(params->getHollow() / SHEAR_QUANTA);
	mesgsys->addU8Fast(_PREHASH_ProfileHollow, tempU8);

	return true;
}

bool LLVolumeMessage::packProfileParams(
	const LLProfileParams* params,
	LLDataPacker &dp)
{
	// Default to cylinder
	static LLProfileParams defaultparams(LL_PCODE_PROFILE_CIRCLE, U8(0), U8(0), U8(0));
	
	if (!params)
		params = &defaultparams;
	
	U8 tempU8;
	tempU8 = params->getCurveType();
	dp.packU8(tempU8, "Curve");

	tempU8 = (U8) llround( params->getBegin() / CUT_QUANTA);
	dp.packU8(tempU8, "Begin");

	tempU8 = 200 - (U8) llround(params->getEnd() / CUT_QUANTA);
	dp.packU8(tempU8, "End");

	tempU8 = (S8) llround(params->getHollow() / SHEAR_QUANTA);
	dp.packU8(tempU8, "Hollow");
	return true;
}

bool LLVolumeMessage::unpackProfileParams(
	LLProfileParams* params,
	LLMessageSystem* mesgsys,
	char* block_name,
	S32 block_num)
{
	bool ok = true;
	U8 temp_u8;
	F32 temp_f32;

	mesgsys->getU8Fast(block_name, _PREHASH_ProfileCurve, temp_u8, block_num);
	params->setCurveType(temp_u8);

	mesgsys->getU8Fast(block_name, _PREHASH_ProfileBegin, temp_u8, block_num);
	temp_f32 = temp_u8 * CUT_QUANTA;
	if (temp_f32 > 1.f)
	{
		llwarns << "Profile begin out of range: " << temp_f32
			<< ". Clamping to 0.0." << llendl;
		temp_f32 = 0.f;
		ok = false;
	}
	params->setBegin(temp_f32);

	mesgsys->getU8Fast(block_name, _PREHASH_ProfileEnd, temp_u8, block_num);
	temp_f32 = temp_u8 * CUT_QUANTA;
	if (temp_f32 > 1.f)
	{
		llwarns << "Profile end out of range: " << 1.f - temp_f32
			<< ". Clamping to 1.0." << llendl;
		temp_f32 = 1.f;
		ok = false;
	}
	params->setEnd(1.f - temp_f32);

	mesgsys->getU8Fast(block_name, _PREHASH_ProfileHollow, temp_u8, block_num);
	temp_f32 = temp_u8 * SCALE_QUANTA;
	if (temp_f32 > 1.f)
	{
		llwarns << "Profile hollow out of range: " << temp_f32
			<< ". Clamping to 0.0." << llendl;
		temp_f32 = 0.f;
		ok = false;
	}
	params->setHollow(temp_f32);

	/*
	llinfos << "Unpacking Profile Block " << block_num << llendl;
	llinfos << "Curve:     " << (U32)getCurve() << llendl;
	llinfos << "Begin:     " << getBegin() << llendl;
	llinfos << "End:     " << getEnd() << llendl;
	llinfos << "Hollow:     " << getHollow() << llendl;
	*/
	return ok;

}

bool LLVolumeMessage::unpackProfileParams(
	LLProfileParams* params,
	LLDataPacker &dp)
{
	bool ok = true;
	U8 temp_u8;
	F32 temp_f32;

	dp.unpackU8(temp_u8, "Curve");
	params->setCurveType(temp_u8);

	dp.unpackU8(temp_u8, "Begin");
	temp_f32 = temp_u8 * CUT_QUANTA;
	if (temp_f32 > 1.f)
	{
		llwarns << "Profile begin out of range: " << temp_f32 << llendl;
		llwarns << "Clamping to 0.0" << llendl;
		temp_f32 = 0.f;
		ok = false;
	}
	params->setBegin(temp_f32);

	dp.unpackU8(temp_u8, "End");
	temp_f32 = temp_u8 * CUT_QUANTA;
	if (temp_f32 > 1.f)
	{
		llwarns << "Profile end out of range: " << 1.f - temp_f32 << llendl;
		llwarns << "Clamping to 1.0" << llendl;
		temp_f32 = 1.f;
		ok = false;
	}
	params->setEnd(1.f - temp_f32);

	dp.unpackU8(temp_u8, "Hollow");
	temp_f32 = temp_u8 * SCALE_QUANTA;
	if (temp_f32 > 1.f)
	{
		llwarns << "Profile hollow out of range: " << temp_f32 << llendl;
		llwarns << "Clamping to 0.0" << llendl;
		temp_f32 = 0.f;
		ok = false;
	}
	params->setHollow(temp_f32);

	return ok;
}

//============================================================================

// Quantization:
// For cut begin, range is 0 to 1, quanta is 0.005, 0 maps to 0
// For cut end, range is 0 to 1, quanta is 0.005, 1 maps to 0
// For scale, range is 0 to 1, quanta is 0.01, 0 maps to 0, 1 maps to 100
// For shear, range is -0.5 to 0.5, quanta is 0.01, 0 maps to 0
// For taper, range is -1 to 1, quanta is 0.01, 0 maps to 0
bool LLVolumeMessage::packPathParams(
	const LLPathParams* params,
	LLMessageSystem *mesgsys)
{
	// Default to cylinder with no cut, top same size as bottom, no shear, no twist
	static LLPathParams defaultparams(LL_PCODE_PATH_LINE, U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), 0);
	if (!params)
		params = &defaultparams;
	
	U8 curve = params->getCurveType();
	mesgsys->addU8Fast(_PREHASH_PathCurve, curve);

	U8 begin = (U8) llround(params->getBegin() / SCALE_QUANTA);
	mesgsys->addU8Fast(_PREHASH_PathBegin, begin);

	U8 end = 100 - (U8) llround(params->getEnd() / SCALE_QUANTA);
	mesgsys->addU8Fast(_PREHASH_PathEnd, end);

	// Avoid truncation problem with direct F32->U8 cast.
	// (e.g., (U8) (0.50 / 0.01) = (U8) 49.9999999 = 49 not 50.

	U8 pack_scale_x = 200 - (U8) llround(params->getScaleX() / SCALE_QUANTA);
	mesgsys->addU8Fast(_PREHASH_PathScaleX, pack_scale_x );

	U8 pack_scale_y = 200 - (U8) llround(params->getScaleY() / SCALE_QUANTA);
	mesgsys->addU8Fast(_PREHASH_PathScaleY, pack_scale_y );

	U8 pack_shear_x = (U8) llround(params->getShearX() / SHEAR_QUANTA);
	mesgsys->addU8Fast(_PREHASH_PathShearX, pack_shear_x );

	U8 pack_shear_y = (U8) llround(params->getShearY() / SHEAR_QUANTA);
	mesgsys->addU8Fast(_PREHASH_PathShearY, pack_shear_y );

	S8 twist = (S8) llround(params->getTwist() / SCALE_QUANTA);
	mesgsys->addS8Fast(_PREHASH_PathTwist, twist);

	S8 twist_begin = (S8) llround(params->getTwistBegin() / SCALE_QUANTA);
	mesgsys->addS8Fast(_PREHASH_PathTwistBegin, twist_begin);

	S8 radius_offset = (S8) llround(params->getRadiusOffset() / SCALE_QUANTA);
	mesgsys->addS8Fast(_PREHASH_PathRadiusOffset, radius_offset);

	S8 taper_x = (S8) llround(params->getTaperX() / TAPER_QUANTA);
	mesgsys->addS8Fast(_PREHASH_PathTaperX, taper_x);

	S8 taper_y = (S8) llround(params->getTaperY() / TAPER_QUANTA);
	mesgsys->addS8Fast(_PREHASH_PathTaperY, taper_y);

	U8 revolutions = (U8) llround( (params->getRevolutions() - 1.0f) / REV_QUANTA);
	mesgsys->addU8Fast(_PREHASH_PathRevolutions, revolutions);

	S8 skew = (S8) llround(params->getSkew() / SCALE_QUANTA);
	mesgsys->addS8Fast(_PREHASH_PathSkew, skew);

	return true;
}

bool LLVolumeMessage::packPathParams(
	const LLPathParams* params,
	LLDataPacker &dp)
{
	// Default to cylinder with no cut, top same size as bottom, no shear, no twist
	static LLPathParams defaultparams(LL_PCODE_PATH_LINE, U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), U8(0), 0);
	if (!params)
		params = &defaultparams;
	
	U8 curve = params->getCurveType();
	dp.packU8(curve, "Curve");

	U8 begin = (U8) llround(params->getBegin() / SCALE_QUANTA);
	dp.packU8(begin, "Begin");

	U8 end = 100 - (U8) llround(params->getEnd() / SCALE_QUANTA);
	dp.packU8(end, "End");

	// Avoid truncation problem with direct F32->U8 cast.
	// (e.g., (U8) (0.50 / 0.01) = (U8) 49.9999999 = 49 not 50.

	U8 pack_scale_x = 200 - (U8) llround(params->getScaleX() / SCALE_QUANTA);
	dp.packU8(pack_scale_x, "ScaleX");

	U8 pack_scale_y = 200 - (U8) llround(params->getScaleY() / SCALE_QUANTA);
	dp.packU8(pack_scale_y, "ScaleY");

	S8 pack_shear_x = (S8) llround(params->getShearX() / SHEAR_QUANTA);
	dp.packU8(*(U8 *)&pack_shear_x, "ShearX");

	S8 pack_shear_y = (S8) llround(params->getShearY() / SHEAR_QUANTA);
	dp.packU8(*(U8 *)&pack_shear_y, "ShearY");

	S8 twist = (S8) llround(params->getTwist() / SCALE_QUANTA);
	dp.packU8(*(U8 *)&twist, "Twist");
	
	S8 twist_begin = (S8) llround(params->getTwistBegin() / SCALE_QUANTA);
	dp.packU8(*(U8 *)&twist_begin, "TwistBegin");

	S8 radius_offset = (S8) llround(params->getRadiusOffset() / SCALE_QUANTA);
	dp.packU8(*(U8 *)&radius_offset, "RadiusOffset");

	S8 taper_x = (S8) llround(params->getTaperX() / TAPER_QUANTA);
	dp.packU8(*(U8 *)&taper_x, "TaperX");

	S8 taper_y = (S8) llround(params->getTaperY() / TAPER_QUANTA);
	dp.packU8(*(U8 *)&taper_y, "TaperY");

	U8 revolutions = (U8) llround( (params->getRevolutions() - 1.0f) / REV_QUANTA);
	dp.packU8(*(U8 *)&revolutions, "Revolutions");

	S8 skew = (S8) llround(params->getSkew() / SCALE_QUANTA);
	dp.packU8(*(U8 *)&skew, "Skew");

	return true;
}

bool LLVolumeMessage::unpackPathParams(
	LLPathParams* params,
	LLMessageSystem* mesgsys,
	char* block_name,
	S32 block_num)
{
	U8 curve;
	mesgsys->getU8Fast(block_name, _PREHASH_PathCurve, curve, block_num);
	params->setCurveType(curve);

	U8 begin;
	mesgsys->getU8Fast(block_name, _PREHASH_PathBegin, begin, block_num);
	params->setBegin((F32)(begin * SCALE_QUANTA));

	U8 end;
	mesgsys->getU8Fast(block_name, _PREHASH_PathEnd, end, block_num);
	params->setEnd((F32)((100 - end) * SCALE_QUANTA));

	U8 pack_scale_x, pack_scale_y;
	mesgsys->getU8Fast(block_name, _PREHASH_PathScaleX, pack_scale_x, block_num);
	mesgsys->getU8Fast(block_name, _PREHASH_PathScaleY, pack_scale_y, block_num);
	F32 x = (F32) (200 - pack_scale_x) * SCALE_QUANTA;
	F32 y = (F32) (200 - pack_scale_y) * SCALE_QUANTA;
	params->setScale( x, y );

	S8 shear_x_quant, shear_y_quant;
	mesgsys->getS8Fast(block_name, _PREHASH_PathShearX, shear_x_quant, block_num);
	mesgsys->getS8Fast(block_name, _PREHASH_PathShearY, shear_y_quant, block_num);
	F32 shear_x = (F32) shear_x_quant * SHEAR_QUANTA;
	F32 shear_y = (F32) shear_y_quant * SHEAR_QUANTA;
	params->setShear( shear_x, shear_y );

	S8 twist;
	mesgsys->getS8Fast(block_name, _PREHASH_PathTwist, twist, block_num );
	params->setTwist((F32)(twist * SCALE_QUANTA));

	S8 twist_begin;
	mesgsys->getS8Fast(block_name, _PREHASH_PathTwistBegin, twist_begin, block_num );
	params->setTwistBegin((F32)(twist_begin * SCALE_QUANTA));
	
	S8 radius_offset;
	mesgsys->getS8Fast(block_name, _PREHASH_PathRadiusOffset, radius_offset, block_num );
	params->setRadiusOffset((F32)(radius_offset * SCALE_QUANTA));

	S8 taper_x_quant, taper_y_quant;
	mesgsys->getS8Fast(block_name, _PREHASH_PathTaperX, taper_x_quant, block_num );
	mesgsys->getS8Fast(block_name, _PREHASH_PathTaperY, taper_y_quant, block_num );
	F32 taper_x = (F32)(taper_x_quant * TAPER_QUANTA);
	F32 taper_y = (F32)(taper_y_quant * TAPER_QUANTA);
	params->setTaper( taper_x, taper_y );

	U8 revolutions;
	mesgsys->getU8Fast(block_name, _PREHASH_PathRevolutions, revolutions, block_num );
	params->setRevolutions((F32)(revolutions * REV_QUANTA + 1.0f));

	S8 skew;
	mesgsys->getS8Fast(block_name, _PREHASH_PathSkew, skew, block_num );
	params->setSkew((F32)(skew * SCALE_QUANTA));

/*
	llinfos << "Unpacking Path Block " << block_num << llendl;
	llinfos << "Curve:     " << (U32)params->getCurve() << llendl;
	llinfos << "Begin:     " << params->getBegin() << llendl;
	llinfos << "End:     " << params->getEnd() << llendl;
	llinfos << "Scale:     " << params->getScale() << llendl;
	llinfos << "Twist:     " << params->getTwist() << llendl;
*/

	return true;

}

bool LLVolumeMessage::unpackPathParams(LLPathParams* params, LLDataPacker &dp)
{
	U8 value;
	S8 svalue;
	dp.unpackU8(value, "Curve");
	params->setCurveType( value );

	dp.unpackU8(value, "Begin");
	params->setBegin((F32)(value * SCALE_QUANTA));

	dp.unpackU8(value, "End");
	params->setEnd((F32)((100 - value) * SCALE_QUANTA));

	dp.unpackU8(value, "ScaleX");
	F32 x = (F32) (200 - value) * SCALE_QUANTA;
	dp.unpackU8(value, "ScaleY");
	F32 y = (F32) (200 - value) * SCALE_QUANTA;
	params->setScale( x, y );

	dp.unpackU8(value, "ShearX");
	svalue = *(S8 *)&value;
	F32 shear_x = (F32) svalue * SHEAR_QUANTA;
	dp.unpackU8(value, "ShearY");
	svalue = *(S8 *)&value;
	F32 shear_y = (F32) svalue * SHEAR_QUANTA;
	params->setShear( shear_x, shear_y );

	dp.unpackU8(value, "Twist");
	svalue = *(S8 *)&value;
	params->setTwist((F32)(svalue * SCALE_QUANTA));

	dp.unpackU8(value, "TwistBegin");
	svalue = *(S8 *)&value;
	params->setTwistBegin((F32)(svalue * SCALE_QUANTA));

	dp.unpackU8(value, "RadiusOffset");
	svalue = *(S8 *)&value;
	params->setRadiusOffset((F32)(svalue * SCALE_QUANTA));

	dp.unpackU8(value, "TaperX");
	svalue = *(S8 *)&value;
	params->setTaperX((F32)(svalue * TAPER_QUANTA));

	dp.unpackU8(value, "TaperY");
	svalue = *(S8 *)&value;
	params->setTaperY((F32)(svalue * TAPER_QUANTA));

	dp.unpackU8(value, "Revolutions");
	params->setRevolutions((F32)(value * REV_QUANTA + 1.0f));

	dp.unpackU8(value, "Skew");
	svalue = *(S8 *)&value;
	params->setSkew((F32)(svalue * SCALE_QUANTA));

	return true;
}

//============================================================================

// static
bool LLVolumeMessage::constrainVolumeParams(LLVolumeParams& params)
{
	bool ok = true;

	// This is called immediately after an unpack. feed the raw data
	// through the checked setters to constraint it to a valid set of
	// volume params.
	ok &= params.setType(
		params.getProfileParams().getCurveType(),
		params.getPathParams().getCurveType());
	ok &= params.setBeginAndEndS(
		params.getProfileParams().getBegin(),
		params.getProfileParams().getEnd());
	ok &= params.setBeginAndEndT(
		params.getPathParams().getBegin(),
		params.getPathParams().getEnd());
	ok &= params.setHollow(params.getProfileParams().getHollow());
	ok &= params.setTwistBegin(params.getPathParams().getTwistBegin());
	ok &= params.setTwistEnd(params.getPathParams().getTwistEnd());
	ok &= params.setRatio(
		params.getPathParams().getScaleX(),
		params.getPathParams().getScaleY());
	ok &= params.setShear(
		params.getPathParams().getShearX(),
		params.getPathParams().getShearY());
	ok &= params.setTaper(
		params.getPathParams().getTaperX(),
		params.getPathParams().getTaperY());
	ok &= params.setRevolutions(params.getPathParams().getRevolutions());
	ok &= params.setRadiusOffset(params.getPathParams().getRadiusOffset());
	ok &= params.setSkew(params.getPathParams().getSkew());
	if(!ok)
	{
		llwarns << "LLVolumeMessage::constrainVolumeParams() - "
			<< "forced to constrain incoming volume params." << llendl;
	}
	return ok;
}

bool LLVolumeMessage::packVolumeParams(const LLVolumeParams* params, LLMessageSystem *mesgsys)
{
	// llinfos << "pack volume" << llendl;
	if (params)
	{
		packPathParams(&params->getPathParams(), mesgsys);
		packProfileParams(&params->getProfileParams(), mesgsys);
	}
	else
	{
		packPathParams(0, mesgsys);
		packProfileParams(0, mesgsys);
	}
	return true;
}

bool LLVolumeMessage::packVolumeParams(const LLVolumeParams* params, LLDataPacker &dp)
{
	// llinfos << "pack volume" << llendl;
	if (params)
	{
		packPathParams(&params->getPathParams(), dp);
		packProfileParams(&params->getProfileParams(), dp);
	}
	else
	{
		packPathParams(0, dp);
		packProfileParams(0, dp);
	}
	return true;
}

bool LLVolumeMessage::unpackVolumeParams(
	LLVolumeParams* params,
	LLMessageSystem* mesgsys,
	char* block_name,
	S32 block_num)
{
	bool ok = true;
	ok &= unpackPathParams(
		&params->getPathParams(),
		mesgsys,
		block_name,
		block_num);
	ok &= unpackProfileParams(
		&params->getProfileParams(),
		mesgsys,
		block_name,
		block_num);
	ok &= constrainVolumeParams(*params);
		
	return ok;
}

bool LLVolumeMessage::unpackVolumeParams(
	LLVolumeParams* params,
	LLDataPacker &dp)
{
	bool ok = true;
	ok &= unpackPathParams(&params->getPathParams(), dp);
	ok &= unpackProfileParams(&params->getProfileParams(), dp);
	ok &= constrainVolumeParams(*params);
	return ok;
}

//============================================================================
