__kernel void getVoisins(
    __global int* voisins,
    int radius,
    int cote)
{
    uint gid = get_global_id(0);  
    int na = cote * cote * cote;
    int currentZ = gid  % cote;
    int tempIndex = gid / cote;
    int currentY = tempIndex % cote;
    tempIndex = tempIndex / cote;
    int currentX = tempIndex % cote;
    int nombreVoisins = (2 * radius + 1) * (2 * radius + 1) * (2 * radius + 1)   - 1;  
    int i = 0;
    for (int z = - radius; z <(int) radius + 1; z++){
      for (int y = - radius; y < (int) radius + 1; y++){
        for (int x = - radius; x < (int)radius + 1; x++){
          if (x != 0 || y != 0 || z != 0){
            int indexVoisin = (( currentX + x) * cote * cote
             + (currentY + y)  * cote + ( currentZ + z)) % na;
             if (indexVoisin < 0){
               indexVoisin = na + indexVoisin;
             }
            voisins[gid * nombreVoisins + i] = indexVoisin;
            i++;
          }
        }
      }
    }
}
