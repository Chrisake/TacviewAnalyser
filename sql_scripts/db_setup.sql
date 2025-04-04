CREATE TABLE Pilot (
    Id SERIAL PRIMARY KEY,
    Username VARCHAR(255) NOT NULL
);

CREATE UNIQUE INDEX idx_pilot_username ON Pilot(Username);

CREATE TABLE ServerFile (
    Id SERIAL PRIMARY KEY,
    Filename VARCHAR(255) NOT NULL,
    FilePath VARCHAR(255) NOT NULL
);

CREATE TABLE TacFile (
    Id SERIAL PRIMARY KEY,
    StartTime TIMESTAMP NOT NULL,
    File INT NOT NULL,
    FOREIGN KEY (File) REFERENCES ServerFile(Id)
);


CREATE TABLE PilotRun (
    Id SERIAL PRIMARY KEY,
    Team VARCHAR(10) CHECK (Team IN ('Red', 'Blue')),
    PilotId INT NOT NULL,
    StartTime TIMESTAMP NOT NULL,
    StartTimeRel INTERVAL NOT NULL,
    EndTime TIMESTAMP NOT NULL,
    EndTimeRel INTERVAL NOT NULL,
    TacFileId INT NOT NULL,
    PositionFile INT,
    FOREIGN KEY (PilotId) REFERENCES Pilot(Id),
    FOREIGN KEY (TacFileId) REFERENCES TacFile(Id),
    FOREIGN KEY (PositionFile) REFERENCES ServerFile(Id)
);

CREATE INDEX idx_pilotrun_pilotid ON PilotRun(PilotId);
CREATE INDEX idx_pilotrun_tacfile ON PilotRun(TacFileId);

CREATE TABLE Missile (
    Id SERIAL PRIMARY KEY,
    Type VARCHAR(255) NOT NULL,
    Team VARCHAR(10) CHECK (Team IN ('Red', 'Blue')),
    LauncherId INT,
    TargetId INT,
    StartTime TIMESTAMP NOT NULL,
    StartTimeRel INTERVAL NOT NULL,
    EndTime TIMESTAMP NOT NULL,
    EndTimeRel INTERVAL NOT NULL,
    TacFileId INT NOT NULL,
    PositionFile INT,
    FOREIGN KEY (LauncherId) REFERENCES PilotRun(Id),
    FOREIGN KEY (TargetId) REFERENCES PilotRun(Id),
    FOREIGN KEY (TacFileId) REFERENCES TacFile(Id),
    FOREIGN KEY (PositionFile) REFERENCES ServerFile(Id)
);

CREATE INDEX idx_missile_tacfile ON Missile(TacFileId);