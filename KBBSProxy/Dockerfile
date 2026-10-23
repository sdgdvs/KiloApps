FROM node:20-alpine
WORKDIR /usr/src/app
COPY package*.json ./
RUN npm ci --omit=dev --ignore-scripts
COPY telnet-proxy.js ./
USER node
EXPOSE 8080
CMD [ "node", "telnet-proxy.js" ]
