#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

#include <stddef.h>

static inline void prefetch( const void* x ) { ; }

#ifndef offsetof
#  define offsetof( TYPE, MEMBER ) ( ( size_t )&( ( TYPE* )0 )->MEMBER )
#endif

/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized list entries.
 */
#define LIST_POISON1 ( ( void* )0x00100100 )
#define LIST_POISON2 ( ( void* )0x00200200 )

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT( name ) { &( name ), &( name ) }

#define LIST_HEAD( name ) struct list_head name = LIST_HEAD_INIT( name )

#define INIT_LIST_HEAD( ptr )                                                                                                              \
    do {                                                                                                                                   \
        ( ptr )->next = ( ptr );                                                                                                           \
        ( ptr )->prev = ( ptr );                                                                                                           \
    } while ( 0 )

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add( struct list_head* new_entry, struct list_head* prev, struct list_head* next )
{
    next->prev = new_entry;
    new_entry->next = next;
    new_entry->prev = prev;
    prev->next = new_entry;
}

/**
 * list_add - add a new entry
 * @new_entry: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add( struct list_head* new_entry, struct list_head* head ) { __list_add( new_entry, head, head->next ); }

/**
 * list_add_tail - add a new entry
 * @new_entry: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail( struct list_head* new_entry, struct list_head* head ) { __list_add( new_entry, head->prev, head ); }

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del( struct list_head* prev, struct list_head* next )
{
    next->prev = prev;
    prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void list_del( struct list_head* entry )
{
    __list_del( entry->prev, entry->next );
    entry->next = ( struct list_head* )LIST_POISON1;
    entry->prev = ( struct list_head* )LIST_POISON2;
}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry( ptr, type, member ) container_of( ptr, type, member )

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry( pos, head, member )                                                                                           \
    for ( pos = list_entry( ( head )->next, typeof( *pos ), member ); prefetch( pos->member.next ), &pos->member != ( head );              \
          pos = list_entry( pos->member.next, typeof( *pos ), member ) )

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop counter.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe( pos, n, head, member )                                                                                   \
    for ( pos = list_entry( ( head )->next, typeof( *pos ), member ), n = list_entry( pos->member.next, typeof( *pos ), member );          \
          &pos->member != ( head ); pos = n, n = list_entry( n->member.next, typeof( *n ), member ) )

/**
 * list_for_each_entry_safe_reverse - iterate backwards over list of given type safe against removal of list entry.
 * @pos:	the type * to use as a loop counter.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe_reverse( pos, n, head, member )                                                                           \
    for ( pos = list_entry( ( head )->prev, typeof( *pos ), member ), n = list_entry( pos->member.prev, typeof( *pos ), member );          \
          prefetch( pos->member.prev ), &pos->member != ( head ); pos = n, n = list_entry( n->member.prev, typeof( *n ), member ) )

#endif /* _LINUX_LIST_H */
