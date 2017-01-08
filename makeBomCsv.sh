#/bin/bash

grep -E '^\|[^\|]*\| [1-9]' BOM.md | sed -r "s/ *\| */,/g" | sed -r "s/,([^,]*),([^,]*),([^,]*).*/\1\t\2\t\3/" |  sort > BOM.tsv

TOTAL=`cut -f2,3 BOM.tsv | sed -r "s/(.*)\t(.*)/ (\1 * \2) /g" | sed -r "s/  */ /g" | tr '\n' '+' | sed -r "s/^ (.*) \+$/\1\n/" | bc`

echo "Price total: $TOTAL"
