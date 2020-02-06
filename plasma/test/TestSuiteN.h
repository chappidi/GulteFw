#pragma once
#include "TestSuite.h"
////////////////////////////////////////////////////////////////////////////////////////////
// 
struct NewOrder {
	TestSuite& _ts;
	uint32_t id;
	NewOrder(TestSuite& ts) :_ts(ts) {
		id = _ts.new_order();
	}
	void pending() {
		_ts.pending_ack(id);
	}
	void accept() {
		_ts.ack(id);
	}
	void reject() {
		_ts.reject(id);
	}
	void resent() {
		_ts.resend_nos(id);
	}
	void fill(double qty) {
		_ts.fill(id, qty);
	}
	void done() {
		_ts.done(id);
	}
	void canceled() {
		_ts.cxld(id);
	}
	void resend() {
		_ts.resend_nos(id);
	}
	void check() {
		
	}
};