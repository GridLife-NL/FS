/** 
 * @file lltransfersourceasset.cpp
 * @brief Transfer system for sending an asset.
 *
 * Copyright (c) 2006-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

#include "linden_common.h"

#include "lltransfersourceasset.h"

#include "llerror.h"
#include "message.h"
#include "lldatapacker.h"
#include "lldir.h"
#include "llvfile.h"

LLTransferSourceAsset::LLTransferSourceAsset(const LLUUID &request_id, const F32 priority) :
	LLTransferSource(LLTST_ASSET, request_id, priority),
	mGotResponse(FALSE),
	mCurPos(0)
{
}

LLTransferSourceAsset::~LLTransferSourceAsset()
{
}


void LLTransferSourceAsset::initTransfer()
{
	if (gAssetStorage)
	{
		// *HACK: asset transfers will only be coming from the viewer
		// to the simulator. This is subset of assets we allow to be
		// simply pulled straight from the asset system.
		// *FIX: Make this list smaller.
		LLUUID* tidp;
		switch(mParams.getAssetType())
		{
		case LLAssetType::AT_SOUND:
		case LLAssetType::AT_LANDMARK:
		case LLAssetType::AT_CLOTHING:
		case LLAssetType::AT_BODYPART:
		case LLAssetType::AT_GESTURE:
		case LLAssetType::AT_ANIMATION:
			tidp = new LLUUID(getID());
			gAssetStorage->getAssetData(
				mParams.getAssetID(),
				mParams.getAssetType(),
				LLTransferSourceAsset::responderCallback,
				tidp,
				FALSE);
			break;
		default:
		llwarns << "Attempted to request blocked asset "
			<< mParams.getAssetID() << ":"
			<< LLAssetType::lookupHumanReadable(mParams.getAssetType())
			<< llendl;
			sendTransferStatus(LLTS_ERROR);
			break;
		}
	}
	else
	{
		llwarns << "Attempted to request asset "
			<< mParams.getAssetID() << ":" << LLAssetType::lookupHumanReadable(mParams.getAssetType())
			<< " without an asset system!" << llendl;
		sendTransferStatus(LLTS_ERROR);
	}
}

F32 LLTransferSourceAsset::updatePriority()
{
	return 0.f;
}

LLTSCode LLTransferSourceAsset::dataCallback(const S32 packet_id,
											const S32 max_bytes,
											U8 **data_handle,
											S32 &returned_bytes,
											BOOL &delete_returned)
{
	//llinfos << "LLTransferSourceAsset::dataCallback" << llendl;
	if (!mGotResponse)
	{
		return LLTS_SKIP;
	}

	LLVFile vf(gAssetStorage->mVFS, mParams.getAssetID(), mParams.getAssetType(), LLVFile::READ);

	if (!vf.getSize())
	{
		// Something bad happened with the asset request!
		return LLTS_ERROR;
	}

	if (packet_id != mLastPacketID + 1)
	{
		llerrs << "Can't handle out of order file transfer yet!" << llendl;
	}

	// grab a buffer from the right place in the file
	if (!vf.seek(mCurPos, 0))
	{
		llwarns << "LLTransferSourceAsset Can't seek to " << mCurPos << " length " << vf.getSize() << llendl;
		llwarns << "While sending " << mParams.getAssetID() << llendl;
		return LLTS_ERROR;
	}
	
	delete_returned = TRUE;
	U8 *tmpp = new U8[max_bytes];
	*data_handle = tmpp;
	if (!vf.read(tmpp, max_bytes))		/* Flawfinder: Ignore */
	{
		// Crap, read failure, need to deal with it.
		delete[] tmpp;
		*data_handle = NULL;
		returned_bytes = 0;
		delete_returned = FALSE;
		return LLTS_ERROR;
	}

	returned_bytes = vf.getLastBytesRead();
	mCurPos += returned_bytes;


	if (vf.eof())
	{
		if (!returned_bytes)
		{
			delete[] tmpp;
			*data_handle = NULL;
			returned_bytes = 0;
			delete_returned = FALSE;
		}
		return LLTS_DONE;
	}

	return LLTS_OK;
}

void LLTransferSourceAsset::completionCallback(const LLTSCode status)
{
	// No matter what happens, all we want to do is close the vfile if
	// we've got it open.
}

BOOL LLTransferSourceAsset::unpackParams(LLDataPacker &dp)
{
	//llinfos << "LLTransferSourceAsset::unpackParams" << llendl;

	return mParams.unpackParams(dp);
}


void LLTransferSourceAsset::responderCallback(LLVFS *vfs, const LLUUID& uuid, LLAssetType::EType type,
											  void *user_data, S32 result)
{
	LLUUID *tidp = ((LLUUID*) user_data);
	LLUUID transfer_id = *(tidp);
	delete tidp;
	tidp = NULL;

	LLTransferSourceAsset *tsap = (LLTransferSourceAsset *)	gTransferManager.findTransferSource(transfer_id);

	if (!tsap)
	{
		llinfos << "Aborting transfer " << transfer_id << " callback, transfer source went away" << llendl;
		return;
	}

	if (result)
	{
		llinfos << "AssetStorage: Error " << gAssetStorage->getErrorString(result) << " downloading uuid " << uuid << llendl;
	}

	LLTSCode status;

	tsap->mGotResponse = TRUE;
	if (LL_ERR_NOERR == result)
	{
		// Everything's OK.
		LLVFile vf(gAssetStorage->mVFS, uuid, type, LLVFile::READ);
		tsap->mSize = vf.getSize();
		status = LLTS_OK;
	}
	else
	{
		// Uh oh, something bad happened when we tried to get this asset!
		switch (result)
		{
		case LL_ERR_ASSET_REQUEST_NOT_IN_DATABASE:
			status = LLTS_UNKNOWN_SOURCE;
			break;
		default:
			status = LLTS_ERROR;
		}
	}

	tsap->sendTransferStatus(status);
}



LLTransferSourceParamsAsset::LLTransferSourceParamsAsset() : LLTransferSourceParams(LLTST_ASSET)
{
}

void LLTransferSourceParamsAsset::setAsset(const LLUUID &asset_id, const LLAssetType::EType asset_type)
{
	mAssetID = asset_id;
	mAssetType = asset_type;
}

void LLTransferSourceParamsAsset::packParams(LLDataPacker &dp) const
{
	dp.packUUID(mAssetID, "AssetID");
	dp.packS32(mAssetType, "AssetType");
}


BOOL LLTransferSourceParamsAsset::unpackParams(LLDataPacker &dp)
{
	S32 tmp_at;

	dp.unpackUUID(mAssetID, "AssetID");
	dp.unpackS32(tmp_at, "AssetType");

	mAssetType = (LLAssetType::EType)tmp_at;

	return TRUE;
}

