def optimized_bubble_sort(arr):
    """
    Sorts a list in ascending order using an optimized version of Bubble Sort.

    This implementation improves the naive approach in two ways:
    1. It stops early if no swaps occur during a pass (meaning the list is already sorted).
    2. It reduces the range of comparison after each pass because the largest
       elements "bubble up" to their correct positions at the end.
    """

    n = len(arr)

    # Traverse through all elements in the list
    for i in range(n):
        # This flag will help us detect if the list is already sorted
        swapped = False

        # After each outer loop iteration, the last i elements
        # are already in place, so we don't need to check them again.
        for j in range(0, n - i - 1):

            # Compare adjacent elements
            if arr[j] > arr[j + 1]:
                # Swap if elements are in the wrong order
                arr[j], arr[j + 1] = arr[j + 1], arr[j]
                swapped = True

        # Optimization #1:
        # If no swaps occurred during this pass,
        # the list is already sorted and we can exit early.
        if not swapped:
            break

    return arr
