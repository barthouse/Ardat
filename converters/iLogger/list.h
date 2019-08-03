#pragma once

#include <stdlib.h>


class ListElement
{
public:

	ListElement *	m_prev;
	ListElement * 	m_next;
};

class List
{
public:

	List(void)
	{
		m_head.m_prev = NULL;
		m_head.m_next = &m_tail;
		m_tail.m_prev = &m_head;
		m_tail.m_next = NULL;
	}

	void Add(ListElement * inElement)
	{
		ListElement * next = m_head.m_next;

		next->m_prev = inElement;
		inElement->m_next = next;

		m_head.m_next = inElement;
		inElement->m_prev = &m_head;
	}

	void Remove(ListElement * inElement)
	{
		inElement->m_next->m_prev = inElement->m_prev;
		inElement->m_prev->m_next = inElement->m_next;
	}

	ListElement	m_head;
	ListElement	m_tail;

};
