const fs = require('fs');
let valves;

const ValveService = {
  getValves: function () {
    let valves = [];
    try {
      valves = JSON.parse(fs.readFileSync('db/valves.json'));
    } catch (error) {
      console.error('Failed to load valves.', error);
    }

    return valves;
  },

  getValve: function (id) {
    return valves.findIndex((element) => element.id === id);
  },

  createValve: function () {
    const newId = valves.length + 1;

    valves.push({ id: newId });
    try {
      fs.writeFileSync('db/valves.json', JSON.stringify(valves));
    } catch (error) {
      console.error('Failed to write valves.', error);
    }

    return newId;
  },

  updateValve: function (valve) {
    const index = valves.findIndex((element) => element.id === valve.id);
    valves[index] = valve;

    try {
      fs.writeFileSync('db/valves.json', JSON.stringify(valves));
    } catch (error) {
      console.error('Failed to update valves.', error);
    }
  },
};

valves = ValveService.getValves();

module.exports = { ValveService };
