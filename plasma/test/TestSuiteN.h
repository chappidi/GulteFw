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
struct CancelOrder {
	TestSuite& _ts;
	uint32_t idX;
	uint32_t id;
	CancelOrder(NewOrder& no) :_ts(no._ts), idX(no.id) {
		id = _ts.cxl_order(idX);
	}
	void pending() {
		_ts.pending_cancel(id, idX);
	}
	void accept() {
		_ts.cxld(id, idX);
	}
	void reject(const string& reason) {
		_ts.cxl_rjt(id, idX, reason);
	}
	void check() {

	}
};

struct ReplaceOrder {
	TestSuite& _ts;
	uint32_t idX;
	uint32_t id;
	ReplaceOrder(NewOrder& no, double qty) :_ts(no._ts), idX(no.id) {
		id = _ts.rpl_order(idX, qty);
	}
	void pending() {
		_ts.pending_replace(id, idX);
	}
	void reject(const string& reason) {
		_ts.rpl_rjt(id, idX, reason);
	}
	void accept() {
		_ts.replaced(id, idX);
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
};