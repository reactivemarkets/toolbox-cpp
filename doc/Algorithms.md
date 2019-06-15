# Algorithms

## Swap

Useful utility for insertion sort.

``` c
void node_swap(Node p, Node q)
{
    Node prev = p->prev;
    Node next = q->next;

    // First link.
    prev->next = q;
    q->prev = prev;

    // Second link.
    q->next = p;
    p->prev = q;

    // Third link.
    p->next = next;
    next->prev = p;
}
```

## Sum Of Squares

A numerically stable algorithm is given below. It also computes the mean. This algorithm is due to
Knuth, who cites Welford.

    sum_sq = 0
    mean = x[1]
    for i in 2 to n:
        sweep = (i - 1.0) / i
        delta = x[i] - mean
        sum_sq += delta * delta * sweep
        mean += delta / i
    end

The following algorithm (in pseudocode) will compute the covariate version with good numerical
stability.

    sum_x2 = 0
    sum_y2 = 0
    sum_xy = 0
    mean_x = x[1]
    mean_y = y[1]
    for i in 2 to n:
        sweep = (i - 1.0) / i
        delta_x = x[i] - mean_x
        delta_y = y[i] - mean_y
        sum_x2 += delta_x * delta_x * sweep
        sum_y2 += delta_y * delta_y * sweep
        sum_xy += delta_x * delta_y * sweep
        mean_x += delta_x / i
        mean_y += delta_y / i
    end
