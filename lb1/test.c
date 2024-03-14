void *program(void *arg) {
  struct Student *student = (struct Student *)arg;
  char *gender = student->gender == 0 ? "Мужской" : "Женский";

  pthread_mutex_lock(&conscienceMutex);
  if (changeGender) {
    // pthread_cond_signal(&changeMale);
    currentGender = student->gender;
    maleConscience = 0;
    femaleConscience = 0;
    changeGender = false;
    printf("Пол - %s\n", currentGender == 0 ? "Мужской" : "Женский");
  }
  pthread_mutex_unlock(&conscienceMutex);

  // printf("Студент %d {%s} пришёл в ванную комнату\n\n", student->id, gender);

  if (student->gender == 0) {
    while (1) {
      pthread_mutex_lock(&conscienceMutex);
      if (changeGender) {
        currentGender = student->gender;
        maleConscience = 0;
        femaleConscience = 0;
        changeGender = false;
        printf("Пол - %s\n", currentGender == 0 ? "Мужской" : "Женский");
      }
      pthread_mutex_unlock(&conscienceMutex);

      if (maleConscience >= MAX_CONSCIENCE) {
        printf("Десяти секундное ожидание - %s пол\n", gender);
        changeGender = true;
        sleep(MAX_WAITING);
      }

      int value;
      sem_getvalue(&cabins, &value);

      if (currentGender == student->gender && value > 0) {
        // if (Mkeys != 0 && Fkeys == NUMBER_OF_CABINS) {
        // Ждём когда освободиться кабинка
        // Возможно и это тут не нужно
        sem_wait(&cabins);

        pthread_mutex_lock(&conscienceMutex);
        maleConscience++;
        pthread_mutex_unlock(&conscienceMutex);
        printf("Мужская Совесть - %d\n", maleConscience);

        int washingTime = rand() % 5 + 1;
        printf("Студент %d {%s} вошёл в ванную комнату на %d секунд\n\n",
               student->id, gender, washingTime);

        sleep(washingTime);

        count++;
        maleCount++;

        printf("Студент %d {%s} вышел из ванной комнаты\n\n", student->id,
               gender);
        // Освобождаем кабинку
        sem_post(&cabins);

        return NULL;
      }
    }
  } else {
    while (1) {
      pthread_mutex_lock(&conscienceMutex);
      if (changeGender) {
        currentGender = student->gender;
        maleConscience = 0;
        femaleConscience = 0;
        changeGender = false;
        printf("Пол - %s\n", currentGender == 0 ? "Мужской" : "Женский");
      }
      pthread_mutex_unlock(&conscienceMutex);

      if (femaleConscience >= MAX_CONSCIENCE) {
        printf("Десяти секундное ожидание - %s пол\n", gender);
        changeGender = true;
        sleep(MAX_WAITING);
      }

      int value;
      sem_getvalue(&cabins, &value);

      if (currentGender == student->gender && value > 0) {
        // if (Fkeys != 0 && Mkeys == NUMBER_OF_CABINS) {
        // Ждём когда освободиться кабинка
        sem_wait(&cabins);

        pthread_mutex_lock(&conscienceMutex);
        femaleConscience++;
        pthread_mutex_unlock(&conscienceMutex);
        printf("Женская Совесть - %d\n\n", femaleConscience);

        int washingTime = rand() % 5 + 1;
        printf("Студент %d {%s} вошёл в ванную комнату на %d секунд\n\n",
               student->id, gender, washingTime);

        sleep(washingTime);

        count++;
        femaleCount++;

        printf("Студент %d {%s} вышел из ванной комнаты\n\n", student->id,
               gender);
        // Освобождаем кабинку
        sem_post(&cabins);

        return NULL;
      }
    }
  }
}