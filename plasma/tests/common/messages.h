#pragma once
#include <plasma_client/NewOrderSingle.h>
#include <plasma_client/OrderCancelRequest.h>
#include <plasma_client/OrderReplaceRequest.h>
#include <plasma_client/OrderStatusRequest.h>
#include <plasma_client/OrderCancelReject.h>
#include <plasma_client/DontKnowTrade.h>
#include <memory>

/////////////////////////////////////////////////////////////////////////
//
//
template<typename T>
struct PROXY : public T {
	std::unique_ptr<char[]> data;
	PROXY(int length = 1024)
		: data(std::make_unique<char[]>(length)) {
		this->wrapAndApplyHeader(data.get(), 0, length);
	}
	PROXY(PROXY<T>&& o) noexcept
		: data(std::move(o.data)) {
		this->wrapAndApplyHeader(data.get(), 0, o.bufferLength());
	}

	PROXY<T>& operator=(const T& rhs) {
		assert(sbeSchemaVersion() == rhs.sbeSchemaVersion());
		memcpy(data.get(), rhs.buffer(), rhs.bufferLength());
		return *this;
	}
	PROXY<T>& operator=(const PROXY<T>& rhs) {
		assert(sbeSchemaVersion() == rhs.sbeSchemaVersion());
		memcpy(data.get(), rhs.buffer(), rhs.bufferLength());
		return *this;
	}
};
