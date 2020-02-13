#pragma once
#include <memory>
#include <cassert>

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

using namespace plasma::client;
struct EOrder;
struct Sink;

struct ISource {
	virtual uint32_t req_seq_no() = 0;
	virtual uint8_t id() = 0;
	virtual const ExecutionReport& execRpt(uint32_t clOrdId) = 0;
	virtual const OrderCancelReject& cxlRjt() = 0;
};

struct ITarget {
	virtual uint32_t clOrdId() = 0;
	virtual Sink& sink() = 0;
	virtual uint8_t id() = 0;
};
