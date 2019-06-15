# Data Structures

Basic data-structures implemented in the C programming language.

## Doubly Linked

### Insert

``` c
void list_insert_before(Node p, Node q)
{
    p->prev = q->prev;
    p->next = q;

    p->prev->next = p;
    p->next->prev = p;
}
```

``` c
void list_insert_after(Node p, Node q)
{
    q->prev = p;
    q->next = p->next;

    q->prev->next = q;
    q->next->prev = q;
}
```

## Tree

## Sift Up

``` c
size_t
sift_up(size_t i)
{
    size_t p;

    // Walk up the tree.
    for (; !root(i); i = p) {
        p = parent(i);
        if (invariant(p, i))
            break;

        // Restore invariant.
        swap(x[p], x[i]);
    }
    return i;
}
```

## Sift Down

``` c
size_t sift_down(size_t i)
{
    size_t c;

    // Walk down the tree.
    for (; (c = left_child(i)) && c <= N; i = c) {

        // Use child with lower value.
        size_t r = c + 1;
        if (r <= N && x[r] < x[c])
            c = r;
        if (invariant(i, c))
            break;

        // Restore invariant.
        swap(x[i], x[c]);
    }
    return i;
}
```
