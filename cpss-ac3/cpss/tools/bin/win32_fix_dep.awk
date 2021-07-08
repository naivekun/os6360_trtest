# Fix dependency file generated by makedepend.exe for VC, BC
# Use as filter
#
function match_external(s) {
  if (s ~ /[A-Z]:\/[Pp][Rr][Oo][Gg][Rr][Aa]/)
    return 1
  if (s ~ /[A-Z]:.*\/[Bb][Oo][Rr][Ll][Aa][Nn][Dd]\//)
    return 1
  if (s ~ /[A-Z]:.*\/[Mm][Ii][Cc][Rr][Oo][Ss]/)
    return 1
  return 0
}
{
  s = $1
  refs = 0
  for (n = 2; n <= NF; n++)
  {
    if (match_external($(n)) == 0)
    {
      s=s " " $(n)
      refs++
    }
  }
  if (refs > 0)
    print s
}